package org.yogpstop.yt;

import java.util.HashMap;

import org.bukkit.Bukkit;
import org.bukkit.ChatColor;
import org.bukkit.command.CommandSender;
import org.bukkit.entity.Player;

public class Afk {
	private static final HashMap<String, String> list = new HashMap<String, String>();;

	static final boolean command(String command, String[] args, Player p) {
		if (command.equals("afk")) {
			if (list.containsKey(p.getName())) {
				list.remove(p.getName());
				Bukkit.broadcastMessage(p.getName()
						.concat(ChatColor.GREEN.toString())
						.concat(" is Online."));
			} else {
				list.put(p.getName(), args.length > 0 ? args[0] : null);
				Bukkit.broadcastMessage(p.getName()
						.concat(ChatColor.RED.toString()).concat(" is AFK."));
				if (list.get(p.getName()) != null)
					Bukkit.broadcastMessage(list.get(p.getName()));
			}
			return true;
		} else if (command.equals("cafk")) {
			if (args.length != 1)
				return false;
			if (list.containsKey(args[0])) {
				p.sendMessage(args[0].concat(ChatColor.RED.toString()).concat(
						" is AFK."));
				if (list.get(args[0]) != null)
					p.sendMessage(list.get(args[0]));
			} else {
				p.sendMessage(args[0].concat(ChatColor.GREEN.toString())
						.concat(" is Online.(or unknown player.)"));
			}
			return true;
		}
		return false;
	}

	static final boolean command(String command, String[] args, CommandSender cs) {
		if (command.equals("cafk")) {
			if (args.length != 1)
				return false;
			if (list.containsKey(args[0])) {
				StringBuilder sb = new StringBuilder();
				sb.append(args[0]);
				sb.append(ChatColor.RED);
				sb.append(" is AFK.");
				if (list.get(args[0]) != null)
					sb.append(list.get(args[0]));
				cs.sendMessage(sb.toString());
			} else {
				cs.sendMessage(args[0].concat(ChatColor.GREEN.toString())
						.concat(" is Online.(or unknown player.)"));
			}
			return true;
		}
		return false;
	}

	static final void quit(String player) {
		list.remove(player);
	}
}
