package com.yogpc.yt;

import java.io.File;
import java.nio.charset.Charset;

import org.bukkit.Bukkit;
import org.bukkit.command.Command;
import org.bukkit.command.CommandSender;
import org.bukkit.event.EventHandler;
import org.bukkit.event.EventPriority;
import org.bukkit.event.Listener;
import org.bukkit.event.player.AsyncPlayerChatEvent;
import org.bukkit.event.player.PlayerJoinEvent;
import org.bukkit.event.player.PlayerQuitEvent;
import org.bukkit.plugin.java.JavaPlugin;

public class YogpstopTweaks extends JavaPlugin implements Listener {
  static final Charset utf8 = Charset.forName("UTF-8");

  @Override
  public void onEnable() {
    final File df = getDataFolder();
    Channel.loadCfg(df);
    Bouyomi.loadCfg(df);
    Bukkit.getPluginManager().registerEvents(this, this);
  }

  @Override
  public boolean onCommand(final CommandSender s, final Command c, final String l,
      final String[] args) {
    final String cs = c.getName().toLowerCase();
    if (Channel.com(cs, args, s))
      return true;
    else if (Bouyomi.com(cs, args, s))
      return true;
    return false;
  }

  @EventHandler
  public void onJoin(final PlayerJoinEvent e) {
    Channel.player(e.getPlayer(), true);
  }

  @EventHandler
  public void onQuit(final PlayerQuitEvent e) {
    Channel.player(e.getPlayer(), false);
  }

  @EventHandler(priority = EventPriority.HIGHEST)
  public void onChat(final AsyncPlayerChatEvent e) {
    if (e.isCancelled())
      return;
    Channel.chat(e);
    if (e.isCancelled())
      return;
    Bouyomi.chat(e.getPlayer(), e.getMessage(), null);
  }
}
