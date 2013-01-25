package org.yogpstop.yt;

import java.util.logging.Logger;

import org.bukkit.Bukkit;
import org.bukkit.plugin.java.JavaPlugin;

public class YogpstopTweaks extends JavaPlugin {
	public static Logger logger;
	public static YogpstopTweaks plugin;
	private static final EventListener el = new EventListener();

	private void init() {
		logger = getLogger();
		plugin = this;
		plugin.getConfig().options().copyDefaults(true);
		plugin.saveConfig();
	}

	@Override
	public void onEnable() {
		init();
		AdvancedShutdown.loadConfiguration(getConfig());
		EncryptionBackup.loadConfiguration(getDataFolder(), getConfig());
		JpConv.loadConfiguration(getDataFolder());
		ShortCommand.loadConfiguration(getDataFolder());
		WebPost.loadConfiguration(getConfig());
		Home.loadConfiguration(getDataFolder());
		IPProtect.loadConfiguration(getDataFolder());
		WebPost.onEnable();
		Bukkit.getPluginManager().registerEvents(el, plugin);
	}

	@Override
	public void onDisable() {
		JpConv.saveConfiguration(getDataFolder());
		Home.saveConfiguration(getDataFolder());
		WebPost.onDisable();
	}
}
