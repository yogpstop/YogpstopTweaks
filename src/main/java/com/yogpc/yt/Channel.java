package com.yogpc.yt;

import java.io.File;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.UUID;
import java.util.WeakHashMap;

import javax.xml.bind.DatatypeConverter;

import org.bukkit.Bukkit;
import org.bukkit.command.CommandSender;
import org.bukkit.configuration.ConfigurationSection;
import org.bukkit.configuration.MemoryConfiguration;
import org.bukkit.configuration.file.FileConfiguration;
import org.bukkit.configuration.file.YamlConfiguration;
import org.bukkit.entity.Player;
import org.bukkit.event.player.AsyncPlayerChatEvent;

public class Channel {
  private static final MessageDigest md;
  static {
    MessageDigest mdt = null;
    try {
      mdt = MessageDigest.getInstance("SHA-512");
    } catch (final NoSuchAlgorithmException e) {
      e.printStackTrace();
    }
    md = mdt;
  }
  private static File cfgf;
  private static final Map<String, Channel> map = new HashMap<String, Channel>();
  private static final Map<UUID, String> def = new HashMap<UUID, String>();
  private final Set<UUID> players = new HashSet<UUID>();
  private final Set<Player> actives = Collections.newSetFromMap(new WeakHashMap<Player, Boolean>());
  private final byte[] pwdig;

  public static void loadCfg(final File df) {
    cfgf = new File(df, "cc.yml");
    final FileConfiguration cfg = YamlConfiguration.loadConfiguration(cfgf);
    def.clear();
    final ConfigurationSection dfs = cfg.getConfigurationSection("defs");
    if (dfs != null)
      for (final Map.Entry<String, Object> e : dfs.getValues(false).entrySet())
        if (e.getValue() instanceof String)
          def.put(UUID.fromString(e.getKey()), (String) e.getValue());
    map.clear();
    final ConfigurationSection cls = cfg.getConfigurationSection("chs");
    if (cls != null)
      for (final Map.Entry<String, Object> e : cls.getValues(false).entrySet())
        if (e.getValue() instanceof ConfigurationSection)
          map.put(e.getKey(), new Channel((ConfigurationSection) e.getValue()));
  }

  private Channel(final ConfigurationSection cfg) {
    final String pw = cfg.getString("pw");
    if (pw != null)
      this.pwdig = DatatypeConverter.parseBase64Binary(pw);
    else
      this.pwdig = null;
    this.players.clear();
    for (final String s : cfg.getStringList("pls"))
      this.players.add(UUID.fromString(s));
    this.actives.clear();
    for (final Player pl : Bukkit.getServer().getOnlinePlayers())
      if (this.players.contains(pl.getUniqueId()))
        this.actives.add(p);
  }

  private static void saveCfg() {
    final YamlConfiguration cfg = new YamlConfiguration();
    final ConfigurationSection dfs = cfg.createSection("defs");
    for (final Map.Entry<UUID, String> d : def.entrySet())
      dfs.set(d.getKey().toString(), d.getValue());
    final ConfigurationSection cls = cfg.createSection("chs");
    for (final Map.Entry<String, Channel> e : map.entrySet()) {
      final Channel c = e.getValue();
      final ConfigurationSection cc = cls.createSection(e.getKey());
      final List<String> ps = new ArrayList<String>();
      for (final UUID pl : c.players)
        ps.add(pl.toString());
      cc.set("pls", ps);
      if (c.pwdig != null)
        cc.set("pw", DatatypeConverter.printBase64Binary(c.pwdig));
    }
    try {
      cfg.save(cfgf);
    } catch (final Exception e1) {
      e1.printStackTrace();
    }
  }

  public static void player(final Player pl, final boolean join) {
    for (final Channel c : map.values())
      if (join && c.players.contains(pl.getUniqueId()))
        c.actives.add(pl);
      else if (!join)
        c.actives.remove(pl);
  }

  public static void chat(final AsyncPlayerChatEvent e) {
    final String s = def.get(e.getPlayer().getUniqueId());
    if (s == null)
      return;
    final Channel c = map.get(s);
    if (c == null)
      return;
    e.setCancelled(true);
    final String m =
        "[" + s + "] "
            + String.format(e.getFormat(), e.getPlayer().getDisplayName(), e.getMessage());
    for (final Player pl : c.actives)
      pl.sendMessage(m);
    // TODO more flexibility
    Bouyomi.chat(e.getPlayer(), e.getMessage(), s);
  }

  // TODO encapsulation
  private static Player p;
  private static Channel ch;

  private static final int REQ_PLAYER = 1;
  private static final int REQ_CHANNEL = 2;
  private static final int REQ_NO_CHANNEL = 4;
  private static final int REQ_PASSWORD = 8 | REQ_CHANNEL;
  private static final int REQ_JOIN = 16 | REQ_CHANNEL | REQ_PLAYER;

  private static final boolean check(final String[] a, final CommandSender cs, final int f) {
    p = null;
    ch = null;
    if ((f & REQ_PLAYER) == REQ_PLAYER) {
      if (!(cs instanceof Player)) {
        cs.sendMessage("そのコマンドはゲーム内から実行してください！");
        return false;
      }
      p = (Player) cs;
    }
    if ((f & REQ_CHANNEL) == REQ_CHANNEL || (f & REQ_NO_CHANNEL) == REQ_NO_CHANNEL) {
      if (a.length < 1) {
        cs.sendMessage("そのコマンドはチャンネル名の指定が必要です！");
        return false;
      }
      ch = map.get(a[0]);
      if ((f & REQ_CHANNEL) == REQ_CHANNEL && ch == null) {
        cs.sendMessage("\"" + a[0] + "\" は存在しないチャンネルです！");
        return false;
      } else if ((f & REQ_NO_CHANNEL) == REQ_NO_CHANNEL && ch != null) {
        cs.sendMessage("\"" + a[0] + "\" は既に存在するチャンネルです！");
        return false;
      }
    }
    if (ch != null && (f & REQ_PASSWORD) == REQ_PASSWORD)
      if (ch.pwdig != null) {
        if (a.length < 2) {
          cs.sendMessage("そのコマンドを実行するには、パスワードが必要です！");
          return false;
        }
        if (!Arrays.equals(md.digest(a[1].getBytes(YogpstopTweaks.utf8)), ch.pwdig)) {
          cs.sendMessage("チャンネル \"" + a[0] + "\" のパスワードが不正です！");
          return false;
        }
      }
    if (ch != null && p != null && (f & REQ_JOIN) == REQ_JOIN)
      if (!ch.actives.contains(p)) {
        cs.sendMessage("そのコマンドを実行するにはチャンネルに参加する必要があります！");
        return false;
      }
    return true;
  }

  public static boolean com(final String m, final String[] a, final CommandSender cs) {
    if ("ch".equals(m)) {
      if (a.length < 1) {
        if (!check(a, cs, REQ_PLAYER))
          return false;
        if (def.remove(p.getUniqueId()) != null)
          saveCfg();
        cs.sendMessage("デフォルトの発言先をサーバー全体に変更しました。");
        return true;
      }
      if (!check(a, cs, REQ_JOIN))
        return false;
      if (!ch.equals(def.put(p.getUniqueId(), a[0])))
        saveCfg();
      cs.sendMessage("デフォルトの発言先を \"" + a[0] + "\" チャンネルに変更しました。");
      return true;
    }
    if ("chl".equals(m)) {
      if (a.length < 1) {
        if (!check(a, cs, REQ_PLAYER))
          return false;
        cs.sendMessage("現在参加中のチャンネル----------");
        for (final Map.Entry<String, Channel> e : map.entrySet())
          if (e.getValue().actives.contains(p))
            cs.sendMessage(e.getKey());
        cs.sendMessage("---------------------------");
        return true;
      }
      if (!check(a, cs, REQ_JOIN))
        return false;
      cs.sendMessage("チャンネルメンバー一覧------------");
      for (final UUID u : ch.players)
        cs.sendMessage(Bukkit.getOfflinePlayer(u).getName());
      cs.sendMessage("---------------------------");
      return true;
    }
    if ("chj".equals(m)) {
      if (!check(a, cs, REQ_PLAYER | REQ_PASSWORD))
        return false;
      if (ch.players.add(p.getUniqueId()))
        saveCfg();
      ch.actives.add(p);
      cs.sendMessage("チャンネル \"" + a[0] + "\" に参加しました。");
      return true;
    }
    if ("chx".equals(m)) {
      if (!check(a, cs, REQ_PLAYER | REQ_CHANNEL))
        return false;
      if (ch.players.remove(p.getUniqueId()))
        saveCfg();
      ch.actives.remove(p);
      cs.sendMessage("チャンネル \"" + a[0] + "\" から退出しました。");
      return true;
    }
    if ("chc".equals(m)) {
      if (!check(a, cs, REQ_NO_CHANNEL))
        return false;
      final MemoryConfiguration mc = new MemoryConfiguration();
      if (a.length > 1)
        mc.set("pw",
            DatatypeConverter.printBase64Binary(md.digest(a[1].getBytes(YogpstopTweaks.utf8))));
      map.put(a[0], new Channel(mc));
      saveCfg();
      cs.sendMessage("チャンネル \"" + a[0] + "\" を作成しました。");
      return true;
    }
    if ("chd".equals(m)) {
      if (!check(a, cs, REQ_PASSWORD))
        return false;
      if (map.remove(a[0]) != null)
        saveCfg();
      cs.sendMessage("チャンネル \"" + a[0] + "\" を削除しました。");
      return true;
    }
    return false;
  }
}
