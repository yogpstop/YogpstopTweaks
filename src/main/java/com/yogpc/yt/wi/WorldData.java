package com.yogpc.yt.wi;

import java.io.File;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.bukkit.Bukkit;
import org.bukkit.GameMode;
import org.bukkit.World;
import org.bukkit.configuration.ConfigurationSection;
import org.bukkit.configuration.file.YamlConfiguration;
import org.bukkit.event.player.PlayerRespawnEvent;
import org.bukkit.permissions.Permissible;
import org.bukkit.permissions.Permission;

public class WorldData {
  private static final Map<String, WorldData> map = new HashMap<String, WorldData>();
  private final List<String> worlds;
  private final GameMode gm;
  private final Permission perm;

  private WorldData(final ConfigurationSection cs, final String s) {
    this.worlds = cs.getStringList("worlds");
    this.gm = GameMode.valueOf(cs.getString("gm", "SURVIVAL"));
    this.perm = cs.getBoolean("reqPerm", false) ? new Permission("com.yogpc.goto." + s) : null;
    if (this.perm != null)
      Bukkit.getPluginManager().addPermission(this.perm);
  }

  public static void loadCfg(final File df) {
    map.clear();
    final ConfigurationSection cfg = YamlConfiguration.loadConfiguration(new File(df, "wg.yml"));
    for (final Map.Entry<String, Object> e : cfg.getValues(false).entrySet())
      map.put(e.getKey(), new WorldData((ConfigurationSection) e.getValue(), e.getKey()));
  }

  static String groupOf(final World w) {
    final String wn = w.getName();
    for (final Map.Entry<String, WorldData> e : map.entrySet())
      if (e.getValue().worlds.contains(wn))
        return e.getKey();
    return wn;
  }

  static String gamemode(final String s) {
    final WorldData wd = map.get(s);
    return wd == null ? "SURVIVAL" : wd.gm.toString();
  }

  static boolean hasPermission(final String g, final Permissible p) {
    final WorldData wd = map.get(g);
    if (wd == null || wd.perm == null)
      return true;
    return p.hasPermission(wd.perm.getName());
  }

  static World getMain(final String g, final World d) {
    final WorldData wd = map.get(g);
    World w = null;
    if (wd != null)
      for (final String e : wd.worlds)
        if ((w = Bukkit.getWorld(e)) != null)
          break;
    if (w == null)
      w = Bukkit.getWorld(g);
    return w == null ? d : w;
  }

  public static void handle(final PlayerRespawnEvent e) {
    final World fw = e.getPlayer().getWorld();
    final String fr = groupOf(fw);
    if (!fr.equals(groupOf(e.getRespawnLocation().getWorld())))
      e.setRespawnLocation(getMain(fr, fw).getSpawnLocation());
  }
}
