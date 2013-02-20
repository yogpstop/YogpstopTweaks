package org.yogpstop.yt;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.util.HashMap;

import org.bukkit.Bukkit;

public class IPProtect {
	private static final HashMap<Long, Byte> allow4 = new HashMap<Long, Byte>();

	// private static HashMap<Long, Byte> allow6 = new HashMap<Long, Byte>();

	static final String prelogin(InetAddress a, String p) {
		if (a instanceof Inet4Address) {
			byte[] base = a.getAddress();
			long y = base[3];
			y += ((long) (base[2] & 0xFF)) << 8;
			y += ((long) (base[1] & 0xFF)) << 16;
			y += ((long) (base[0] & 0xFF)) << 24;
			for (Long l : allow4.keySet()) {
				long c = y >> allow4.get(l);
				if (l.equals(c))
					return null;
			}
			if (Bukkit.getWhitelistedPlayers().contains(
					Bukkit.getOfflinePlayer(p)))
				return null;
			return "You connect from out of Japan.(or using free proxy.)\nThis server allow only Japan IP.\n(or you must visit server HP.)";
		} else if (a instanceof Inet6Address) {
			// TODO
		}
		return "Your IP is unknown.";
	}

	static final void loadConfiguration(File dir) {
		byte x;
		long a;
		allow4.clear();
		File f = new File(dir, "ips.cfg");
		if (!f.exists())
			return;
		try {
			BufferedReader br = new BufferedReader(new FileReader(f));
			String line;
			while ((line = br.readLine()) != null) {
				String[] two = line.split("/");
				if (two.length != 2)
					continue;
				x = (byte) (32 - Byte.parseByte(two[1]));
				String[] four = two[0].split("\\.");
				if (four.length != 4)
					continue;
				a = Long.parseLong(four[3]);
				a += Long.parseLong(four[2]) << 8;
				a += Long.parseLong(four[1]) << 16;
				a += Long.parseLong(four[0]) << 24;
				a = a >> x;
				allow4.put(a, x);
			}
			br.close();
		} catch (NumberFormatException | IOException e) {
			e.printStackTrace();
		}

	}
}
