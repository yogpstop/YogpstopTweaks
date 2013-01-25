package org.yogpstop.yt;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.bukkit.Bukkit;
import org.bukkit.command.CommandSender;
import org.bukkit.entity.Player;

final class ShortCommand {
	private static HashMap<String, ArrayList<String>> commands;

	static void loadConfiguration(File dir) {
		try {
			commands = new HashMap<String, ArrayList<String>>();
			File f = new File(dir, "commands.yml");
			if (!f.exists())
				f.createNewFile();
			BufferedReader b = new BufferedReader(new InputStreamReader(
					new FileInputStream(f)));
			if (!dir.exists())
				dir.mkdir();
			if (!f.exists())
				f.createNewFile();
			String current = "";
			String line;
			while ((line = b.readLine()) != null) {
				line = line.trim();
				if (line.startsWith("-")) {
					commands.get(current).add(line.substring(1));
				} else {
					commands.put(line, new ArrayList<String>());
					current = line;
				}
			}
			b.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	static boolean command(String cmd, String[] args, Player p) {
		if (commands.containsKey(cmd)) {
			for (String s : commands.get(cmd)) {
				p.chat("/".concat(replace(s, args, p)));
			}
			return true;
		}
		return false;
	}

	static boolean command(String cmd, String[] args, CommandSender cs) {
		if (commands.containsKey(cmd)) {
			for (String s : commands.get(cmd)) {
				Bukkit.dispatchCommand(cs, replace(s, args, cs));
			}
			return true;
		}
		return false;
	}

	private static String replace(String cmd, String[] args, CommandSender cs) {
		cmd = cmd.replaceAll("@PLAYER@", cs.getName());
		StringBuilder argsall = new StringBuilder();
		for (String s : args) {
			argsall.append(s);
			argsall.append(" ");
		}
		cmd = cmd.replaceAll("@ARGSALL@", argsall.toString().trim());
		StringBuilder result = new StringBuilder();
		Matcher match = Pattern.compile("@ARGS([0-9]{1,2})@").matcher(cmd);
		int current = 0;
		while (match.find()) {
			result.append(cmd.substring(current, match.start()));
			result.append(args[Integer.parseInt(match.group(1)) - 1]);
			current = match.end();
		}
		result.append(cmd.substring(current));
		return result.toString();
	}

}
