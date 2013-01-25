package org.yogpstop.yt;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLEncoder;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.bukkit.ChatColor;
import org.bukkit.entity.Player;

final class JpConv {
	private static List<String> ignore;
	private static HashMap<String, Byte> type;
	private static final HashMap<String, ArrayList<ArrayList<String>>> clist = new HashMap<String, ArrayList<ArrayList<String>>>();

	private static String getList(String name) {
		StringBuilder info = new StringBuilder();
		StringBuilder ex = new StringBuilder();
		info.append("変換候補\n");
		ex.append(ChatColor.AQUA);
		for (int i = 0; i < clist.get(name).size(); i++) {
			info.append(ChatColor.RED);
			info.append(i);
			info.append("-");
			info.append(ChatColor.WHITE);
			for (int j = 0; j < clist.get(name).get(i).size(); j++) {
				info.append(j);
				info.append(": ");
				info.append(clist.get(name).get(i).get(j));
				info.append(" ");
			}
			info.deleteCharAt(info.length() - 1);
			info.append("\n");
			ex.append(clist.get(name).get(i).get(0));
		}
		info.append(ex);
		return info.toString();
	}

	private static String getChat(String name, String[] cand) {
		StringBuilder output = new StringBuilder();
		if (cand.length != clist.get(name).size()) {
			HashMap<Integer, Integer> special = new HashMap<Integer, Integer>();
			for (int i = 0; i < cand.length; i++) {
				String[] cache = cand[i].split("-");
				if (cache.length == 1)
					return "";
				special.put(Integer.parseInt(cache[0]),
						Integer.parseInt(cache[1]));

			}
			for (int i = 0; i < clist.get(name).size(); i++) {
				output.append(clist.get(name).get(i)
						.get(special.containsKey(i) ? special.get(i) : 0));
			}
		} else {
			for (int i = 0; i < clist.get(name).size(); i++) {
				output.append(clist.get(name).get(i)
						.get(Integer.parseInt(cand[i])));
			}
		}
		clist.remove(name);
		return output.toString();
	}

	static void loadConfiguration(File dir) {
		type = new HashMap<String, Byte>();
		ignore = new ArrayList<String>();
		if (!dir.exists()) {
			dir.mkdir();
		}
		File config = new File(dir, "jp_player");
		if (config.exists()) {
			try {
				BufferedReader br = new BufferedReader(new FileReader(config));
				String line;
				while ((line = br.readLine()) != null) {
					String[] s = line.split(":");
					if (s.length != 2)
						continue;
					type.put(s[0], Byte.parseByte(s[1]));
				}
				br.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}

		{
			File ign = new File(dir, "jp_ignore.cfg");
			try {
				if (!ign.exists()) {
					ign.createNewFile();
				}
				BufferedReader br = new BufferedReader(new FileReader(ign));
				String line;
				while ((line = br.readLine()) != null) {
					if (!line.startsWith("#") && !line.equals("")) {
						ignore.add(line);
					}
				}
				br.close();
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	static void saveConfiguration(File dir) {
		File config = new File(dir, "jp_player");
		try {
			BufferedWriter bw = new BufferedWriter(new FileWriter(config));
			Iterator<String> iter = type.keySet().iterator();
			while (iter.hasNext()) {
				String key = iter.next();
				Byte value = type.get(key);
				bw.write(key + ":" + value.toString());
				bw.newLine();
			}
			bw.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	static void join(String player) {
		if (!type.containsKey(player))
			type.put(player, (byte) 0);
	}

	static String chat(String chat, Player p) {
		if (chat.getBytes().length != chat.length())
			return chat;
		switch (type.get(p.getName())) {
		case 1:// 平仮名
			return processClause(chat);
		case 2:// 漢字(候補無視)
			ArrayList<ArrayList<String>> cache = webConv(processClause(chat));
			StringBuilder output = new StringBuilder();
			for (int i = 0; i < cache.size(); i++) {
				output.append(cache.get(i).get(0));
			}
			return output.toString();
		case 3:// 漢字(候補付き)
			clist.put(p.getName(), webConv(processClause(chat)));
			p.sendMessage(getList(p.getName()));
			return null;
		}
		return chat;

	}

	static boolean command(String cmd, String[] args, Player p) {
		if (cmd.equals(".") && clist.containsKey(p.getName())) {
			p.chat(getChat(p.getName(), args));
			return true;
		} else if (cmd.equals(".,") && clist.containsKey(p.getName())) {
			clist.remove(p.getName());
			p.sendMessage("Successfully remove list.");
			return true;
		} else if (cmd.startsWith(":")) {
			if (args.length > 0) {
				byte hold = type.get(p.getName());
				if (Byte.parseByte(cmd.substring(1)) > 3) {
					type.put(p.getName(), (byte) 0);
				} else {
					type.put(p.getName(), Byte.parseByte(cmd.substring(1)));
				}
				StringBuilder text = new StringBuilder();
				for (String cache : args) {
					text.append(cache);
					text.append(" ");
				}
				text.deleteCharAt(text.length() - 1);
				String s = chat(text.toString(), p);
				if (s != null)
					p.chat(s);
				type.put(p.getName(), hold);
			} else {
				if (Byte.parseByte(cmd.substring(1)) > 3) {
					type.put(p.getName(), (byte) 0);
				} else {
					type.put(p.getName(), Byte.parseByte(cmd.substring(1)));
				}
				p.sendMessage(ChatColor.GREEN + "あなたの変換モードは"
						+ Integer.toString(type.get(p.getName())) + "に変更されました！");

			}
			return true;
		}
		return false;
	}

	private static String processClause(String text) {
		text = text.trim().toLowerCase();
		StringBuilder sb = new StringBuilder();
		Matcher m = Pattern.compile(
				"(zh|zj|zk|zl|z\\,|z\\.|z/|z\\[|z\\]|z\\-|[^a-z])").matcher(
				text);
		int back = 0;
		while (m.find(back)) {
			if (back < m.start()) {
				String c = text.substring(back, m.start());
				if (ignore.contains(c))
					sb.append(c);
				else
					sb.append(romaToHira(c));
			}
			sb.append(data.symbol(m.group()));
			back = m.end();
		}
		if (back < text.length()) {
			String c = text.substring(back);
			if (ignore.contains(c))
				sb.append(c);
			else
				sb.append(romaToHira(c));
		}
		return sb.toString();
	}

	private static ArrayList<ArrayList<String>> webConv(String text) {
		StringBuilder line = new StringBuilder();
		try {
			URL url = new URL(
					"http://www.google.com/transliterate?langpair=ja-Hira|ja&text="
							+ URLEncoder.encode(text, "UTF-8"));
			HttpURLConnection http = (HttpURLConnection) url.openConnection();
			http.setDoOutput(true);
			http.connect();
			InputStreamReader isr = new InputStreamReader(
					http.getInputStream(), "UTF-8");
			int data;
			while ((data = isr.read()) != -1)
				line.append((char) data);
		} catch (IOException e) {
			e.printStackTrace();
		}
		String source = line.toString().replaceAll("\\n", "");
		source = source.trim().substring(1, source.length() - 1);
		while (true) {
			if (source.indexOf("\\u") == -1)
				break;
			String cache = source.substring(source.indexOf("\\u") + 2,
					source.indexOf("\\u") + 6);
			int[] a = new int[1];
			a[0] = Integer.parseInt(cache, 16);
			cache = new String(a, 0, a.length);
			String back = source.substring(0, source.indexOf("\\u"));
			String force = source.substring(source.indexOf("\\u") + 6);
			source = back + cache + force;
		}
		ArrayList<ArrayList<String>> cache = new ArrayList<ArrayList<String>>();
		while (true) {
			if (source.indexOf("[") == -1)
				break;
			ArrayList<String> word = new ArrayList<String>();
			source = source.substring(source.indexOf("[") + 1);
			source = source.substring(source.indexOf("[") + 1);
			String temp = source.substring(1, source.indexOf("]") - 1);
			String[] tempa = temp.split("\",\"");
			for (String s : tempa)
				word.add(s);
			cache.add(word);
			source = source.substring(source.indexOf("]") + 1);
			source = source.substring(source.indexOf("]") + 1);
		}
		return cache;
	}

	private static String romaToHira(String s) {
		StringBuilder sb = new StringBuilder();
		char[] siin = new char[2];
		char[] o_siin = new char[2];
		siin[0] = 0;
		siin[1] = 0;
		boolean lowerCase = false;
		for (char c : s.toCharArray()) {
			o_siin[0] = siin[0];
			o_siin[1] = siin[1];
			if (siin[0] == c) {
				if (c == 'w') {
					sb.append('w');
				} else if (c == 'n') {
					sb.append('ん');
					siin[0] = 0;
					o_siin[0] = 0;
				} else {
					sb.append('っ');
				}
				continue;
			}
			if (siin[0] == 'x' || siin[0] == 'l') {
				siin[0] = 0;
				o_siin[0] = 0;
				lowerCase = true;
			} else {
				if (c == 'a' || c == 'i' || c == 'u' || c == 'e' || c == 'o') {
					sb.append(data.get(
							(siin[0] == 0 ? "" : Character.toString(siin[0]))
									+ (siin[1] == 0 ? "" : Character
											.toString(siin[1])), c, lowerCase));
					siin[0] = 0;
					siin[1] = 0;
					o_siin[0] = 0;
					o_siin[1] = 0;
					lowerCase = false;
				} else if (c == 'y') {
					if (siin[0] == 0) {
						siin[0] = c;
					} else {
						siin[1] = c;
					}
				} else {
					if (siin[0] == 'n' && siin[1] == 0) {
						sb.append('ん');
						siin[0] = 0;
						o_siin[0] = 0;
					}
					if (c == 's') {
						if (siin[0] == 't') {
							siin[1] = c;
						} else {
							siin[0] = c;
						}
					} else if (c == 'h') {
						if (siin[0] == 0) {
							siin[0] = c;
						} else {
							siin[1] = c;
						}
					} else if (c == 'w') {
						if (siin[0] == 0) {
							siin[0] = c;
						} else {
							siin[1] = c;
						}
					} else {
						siin[0] = c;
					}
				}
				if (siin[0] != o_siin[0] && o_siin[0] != 0) {
					sb.append(o_siin[0]);
				}
				if (siin[1] != o_siin[1] && o_siin[1] != 0) {
					sb.append(o_siin[1]);
				}
			}
		}
		if (siin[0] != 0)
			sb.append(siin[0]);
		if (siin[1] != 0)
			sb.append(siin[1]);
		return sb.toString();
	}

	private static final Database data = new Database();

	private static final class Database {
		private final HashMap<String, String[]> siin = new HashMap<String, String[]>();
		private final HashMap<Character, Integer> boin = new HashMap<Character, Integer>();
		private final HashMap<Character, Character> lower = new HashMap<Character, Character>();
		private final HashMap<String, String> symbol = new HashMap<String, String>();

		private Database() {
			put("", "あ", "い", "う", "え", "お");
			put("k", "か", "き", "く", "け", "こ");
			put("s", "さ", "し", "す", "せ", "そ");
			put("t", "た", "ち", "つ", "て", "と");
			put("n", "な", "に", "ぬ", "ね", "の");
			put("h", "は", "ひ", "ふ", "へ", "ほ");
			put("m", "ま", "み", "む", "め", "も");
			put("r", "ら", "り", "る", "れ", "ろ");
			put("g", "が", "ぎ", "ぐ", "げ", "ご");
			put("z", "ざ", "じ", "ず", "ぜ", "ぞ");
			put("d", "だ", "ぢ", "づ", "で", "ど");
			put("b", "ば", "び", "ぶ", "べ", "ぼ");
			put("p", "ぱ", "ぴ", "ぷ", "ぺ", "ぽ");
			put("q", "くぁ", "くぃ", "く", "くぇ", "くぉ");
			put("j", "じゃ", "じ", "じゅ", "じぇ", "じょ");
			put("f", "ふぁ", "ふぃ", "ふ", "ふぇ", "ふぉ");
			put("v", "ヴぁ", "ヴぃ", "ヴ", "ヴぇ", "ヴぉ");
			put("c", "か", "し", "く", "せ", "こ");
			put("w", "わ", "うぃ", "う", "うぇ", "を");
			put("y", "や", "い", "ゆ", "いぇ", "よ");
			put("ts", "つぁ", "つぃ", "つ", "つぇ", "つぉ");
			put("ky", "きゃ", "きぃ", "きゅ", "きぇ", "きょ");
			put("sy", "しゃ", "しぃ", "しゅ", "しぇ", "しょ");
			put("ty", "ちゃ", "ちぃ", "ちゅ", "ちぇ", "ちょ");
			put("ny", "にゃ", "にぃ", "にゅ", "にぇ", "にょ");
			put("hy", "ひゃ", "ひぃ", "ひゅ", "ひぇ", "ひょ");
			put("my", "みゃ", "みぃ", "みゅ", "みぇ", "みょ");
			put("ry", "りゃ", "りぃ", "りゅ", "りぇ", "りょ");
			put("cy", "ちゃ", "ちぃ", "ちゅ", "ちぇ", "ちょ");
			put("gy", "ぎゃ", "ぎぃ", "ぎゅ", "ぎぇ", "ぎょ");
			put("zy", "じゃ", "じぃ", "じゅ", "じぇ", "じょ");
			put("dy", "ぢゃ", "ぢぃ", "ぢゅ", "ぢぇ", "ぢょ");
			put("by", "びゃ", "びぃ", "びゅ", "びぇ", "びょ");
			put("py", "ぴゃ", "ぴぃ", "ぴゅ", "ぴぇ", "ぴょ");
			put("qy", "くゃ", "くぃ", "くゅ", "くぇ", "くょ");
			put("jy", "じゃ", "じぃ", "じゅ", "じぇ", "じょ");
			put("fy", "ふゃ", "ふぃ", "ふゅ", "ふぇ", "ふょ");
			put("vy", "ヴゃ", "ヴぃ", "ヴ", "ヴぇ", "ヴょ");
			put("ch", "ちゃ", "ち", "ちゅ", "ちぇ", "ちょ");
			put("th", "てゃ", "てぃ", "てゅ", "てぇ", "てょ");
			put("wh", "うぁ", "うぃ", "う", "うぇ", "うぉ");
			put("sh", "しゃ", "し", "しゅ", "しぇ", "しょ");
			put("dh", "でゃ", "でぃ", "でゅ", "でぇ", "でょ");
			put("kw", "くぁ", "くぃ", "くぅ", "くぇ", "くぉ");
			put("qw", "くぁ", "くぃ", "くぅ", "くぇ", "くぉ");
			put("gw", "ぐぁ", "ぐぃ", "ぐぅ", "ぐぇ", "ぐぉ");
			put("sw", "すぁ", "すぃ", "すぅ", "すぇ", "すぉ");
			put("tw", "とぁ", "とぃ", "とぅ", "とぇ", "とぉ");
			put("dw", "どぁ", "どぃ", "どぅ", "どぇ", "どぉ");
			put("fw", "ふぁ", "ふぃ", "ふぅ", "ふぇ", "ふぉ");
			put("wy", null, "ゐ", null, "ゑ", null);
			boin.put('a', 0);
			boin.put('i', 1);
			boin.put('u', 2);
			boin.put('e', 3);
			boin.put('o', 4);
			lower.put('あ', 'ぁ');
			lower.put('い', 'ぃ');
			lower.put('う', 'ぅ');
			lower.put('え', 'ぇ');
			lower.put('お', 'ぉ');
			lower.put('や', 'ゃ');
			lower.put('ゆ', 'ゅ');
			lower.put('よ', 'ょ');
			lower.put('つ', 'っ');
			lower.put('か', 'ヵ');
			lower.put('け', 'ヶ');
			lower.put('わ', 'ゎ');
			symbol.put(",", "、");
			symbol.put(".", "。");
			symbol.put("~", "～");
			symbol.put("zh", "←");
			symbol.put("zj", "↓");
			symbol.put("zk", "↑");
			symbol.put("zl", "→");
			symbol.put("z,", "‥");
			symbol.put("z.", "…");
			symbol.put("z[", "『");
			symbol.put("z]", "』");
			symbol.put("z/", "・");
			symbol.put("z-", "～");
			symbol.put("-", "ー");
		}

		private void put(String r, String h1, String h2, String h3, String h4,
				String h5) {
			String[] caches = new String[5];
			caches[0] = h1;
			caches[1] = h2;
			caches[2] = h3;
			caches[3] = h4;
			caches[4] = h5;
			siin.put(r, caches);
		}

		private String get(String r, Character aboin, boolean isLower) {
			String cache = (siin.get(r) == null ? null : siin.get(r)[boin
					.get(aboin)]);
			if (cache == null) {
				cache = r + aboin;
			}
			if (isLower && cache.length() == 1) {
				cache = (lower.get(cache.toCharArray()[0]) == null ? cache
						: lower.get(cache.toCharArray()[0]).toString());
			}
			return cache;
		}

		private String symbol(String s) {
			return (symbol.get(s) == null ? s : symbol.get(s));
		}
	}
}