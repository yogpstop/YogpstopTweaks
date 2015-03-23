package org.yogpstop.yt;

import org.bukkit.event.EventHandler;
import org.bukkit.event.EventPriority;
import org.bukkit.event.Listener;
import org.bukkit.event.player.AsyncPlayerChatEvent;
import org.bukkit.event.player.AsyncPlayerPreLoginEvent;
import org.bukkit.event.player.PlayerCommandPreprocessEvent;
import org.bukkit.event.player.PlayerJoinEvent;
import org.bukkit.event.player.PlayerQuitEvent;
import org.bukkit.event.server.ServerCommandEvent;

public class EventListener implements Listener {

	@EventHandler(priority = EventPriority.LOWEST)
	public void onPlayerCommandPreprocess(PlayerCommandPreprocessEvent e) {
		if (e.isCancelled())
			return;
		String[] splitted = e.getMessage().trim().substring(1).split(" ");
		String command = splitted[0];
		String[] args = new String[splitted.length - 1];
		for (int i = 1; i < splitted.length; i++)
			args[i - 1] = splitted[i];
		e.setCancelled(true);
		if (!JpConv.command(command, args, e.getPlayer()))
			if (!Home.command(command, args, e.getPlayer()))
				if (!Afk.command(command, args, e.getPlayer()))
					if (!ShortCommand.command(command, args, e.getPlayer()))
						e.setCancelled(false);
	}

	@EventHandler(priority = EventPriority.LOWEST)
	public void onServerCommand(ServerCommandEvent e) {
		String[] splitted = e.getCommand().trim().split(" ");
		String command = splitted[0];
		String[] args = new String[splitted.length - 1];
		for (int i = 1; i < splitted.length; i++)
			args[i - 1] = splitted[i];
		if (ShortCommand.command(command, args, e.getSender()))
			e.setCommand("nothingatall");
		else if (Afk.command(command, args, e.getSender()))
			e.setCommand("nothingatall");
		else if (EncryptionBackup.command(command, args, e.getSender()))
			e.setCommand("nothingatall");
	}

	@EventHandler(priority = EventPriority.LOWEST)
	public void onPlayerJoin(PlayerJoinEvent e) {
		EncryptionBackup.join();
		AdvancedShutdown.join();
		JpConv.join(e.getPlayer().getName());
	}

	@EventHandler(priority = EventPriority.LOWEST)
	public void onPlayerQuit(PlayerQuitEvent e) {
		AdvancedShutdown.quit();
		Afk.quit(e.getPlayer().getName());
	}

	@EventHandler(priority = EventPriority.HIGHEST)
	public void onPlayerPreLogin(AsyncPlayerPreLoginEvent e) {
		String s = IPProtect.prelogin(e.getAddress(), e.getName());
		if (s != null)
			e.disallow(AsyncPlayerPreLoginEvent.Result.KICK_WHITELIST, s);
	}

	@EventHandler(priority = EventPriority.HIGHEST)
	public void onPlayerChat(AsyncPlayerChatEvent e) {
		if (e.getMessage().startsWith("/"))
			return;
		if (e.isCancelled())
			return;
		String s = JpConv.chat(e.getMessage(), e.getPlayer());
		if (s != null)
			e.setMessage(s);
		else
			e.setCancelled(true);
	}
}
