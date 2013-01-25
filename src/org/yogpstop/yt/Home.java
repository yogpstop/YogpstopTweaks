package org.yogpstop.yt;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;

import org.bukkit.Bukkit;
import org.bukkit.Location;
import org.bukkit.World;
import org.bukkit.entity.Player;

final class Home {
	private static HashMap<String, Location> list;

	static void loadConfiguration(File dir) {
		list = new HashMap<String, Location>();
		File config = new File(dir, "home_player");
		if (config.exists()) {
			try {
				BufferedReader br = new BufferedReader(new FileReader(config));
				String line;
				while ((line = br.readLine()) != null) {
					String[] s = line.split(":");
					if (s.length != 2)
						continue;
					s[1] = s[1].substring(9, s[1].length() - 1);
					String[] l = s[1].split(",");
					HashMap<String, String> map = new HashMap<String, String>();
					for (String st : l) {

						String[] a = st.split("=");
						if (a.length != 2)
							continue;
						map.put(a[0], a[1]);
					}
					World world = Bukkit.getWorld(map.get("world"));
					if (world == null)
						world = Bukkit.getWorlds().get(0);
					list.put(
							s[0],
							new Location(world,
									Double.parseDouble(map.get("x")), Double
											.parseDouble(map.get("y")), Double
											.parseDouble(map.get("z")), Float
											.parseFloat(map.get("yaw")), Float
											.parseFloat(map.get("pitch"))));
				}
				br.close();
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	static void saveConfiguration(File dir) {
		File config = new File(dir, "home_player");
		try {
			BufferedWriter bw = new BufferedWriter(new FileWriter(config));
			Iterator<String> iter = list.keySet().iterator();
			while (iter.hasNext()) {
				String key = iter.next();
				Location value = list.get(key);
				bw.write(key + ":" + value.toString());
				bw.newLine();
			}
			bw.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	static boolean command(String command, String[] args, Player p) {
		if (command.equals("home")) {
			if (list.containsKey(p.getName()))
				p.teleport(list.get(p.getName()));
			else
				p.teleport(Bukkit.getWorlds().get(0).getSpawnLocation());
			p.sendMessage("Teleport successfully.");
			return true;
		} else if (command.equals("sethome")) {
			list.put(p.getName(), p.getLocation());
			p.sendMessage("Set home successfully.");
			return true;
		}
		return false;
	}
}
