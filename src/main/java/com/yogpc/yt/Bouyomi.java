package com.yogpc.yt;

import java.io.File;
import java.io.IOException;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.HashMap;
import java.util.Map;
import java.util.Queue;
import java.util.concurrent.ConcurrentLinkedQueue;

import org.bukkit.command.CommandSender;
import org.bukkit.configuration.ConfigurationSection;
import org.bukkit.configuration.MemoryConfiguration;
import org.bukkit.configuration.file.FileConfiguration;
import org.bukkit.configuration.file.YamlConfiguration;
import org.bukkit.entity.Player;

public class Bouyomi extends Thread {
  private static File cfgf;
  private static final Bouyomi I = new Bouyomi();
  private static Set def;
  private static final Map<String, Set> map = new HashMap<String, Set>();

  private static final class Set {
    final short tone, type;

    Set(final ConfigurationSection cfg) {
      this.tone = (short) cfg.getInt("tone", -1);
      this.type = (short) cfg.getInt("type", 0);
    }
  }

  public static void loadCfg(final File df) {
    cfgf = new File(df, "bm.yml");
    final FileConfiguration cfg = YamlConfiguration.loadConfiguration(cfgf);
    final ConfigurationSection dfs = cfg.getConfigurationSection("def");
    if (dfs != null)
      def = new Set(dfs);
    final ConfigurationSection cls = cfg.getConfigurationSection("chs");
    if (cls != null)
      for (final Map.Entry<String, Object> e : cls.getValues(false).entrySet())
        if (e.getValue() instanceof ConfigurationSection)
          map.put(e.getKey(), new Set((ConfigurationSection) e.getValue()));
  }

  private static void saveCfg() {
    final YamlConfiguration cfg = new YamlConfiguration();
    if (def != null) {
      final ConfigurationSection dfs = cfg.createSection("def");
      dfs.set("tone", Integer.valueOf(def.tone));
      dfs.set("type", Integer.valueOf(def.type));
    }
    final ConfigurationSection cls = cfg.createSection("chs");
    for (final Map.Entry<String, Set> e : map.entrySet()) {
      final Set c = e.getValue();
      final ConfigurationSection cc = cls.createSection(e.getKey());
      cc.set("tone", Integer.valueOf(c.tone));
      cc.set("type", Integer.valueOf(c.type));
    }
    try {
      cfg.save(cfgf);
    } catch (final Exception e1) {
      e1.printStackTrace();
    }
  }

  public static boolean com(final String m, final String[] arg, final CommandSender cs) {
    if (!m.equals("bm"))
      return false;
    if (arg.length < 1) {
      final boolean c = def != null || !map.isEmpty();
      def = null;
      map.clear();
      if (c)
        saveCfg();
      cs.sendMessage("棒読みを無効化しました！");
      return true;
    }
    final MemoryConfiguration mc = new MemoryConfiguration();
    try {
      if (arg.length > 1)
        mc.set("type", Integer.valueOf(arg[1]));
      if (arg.length > 2)
        mc.set("tone", Integer.valueOf(arg[2]));
    } catch (final Exception e) {
      cs.sendMessage("棒読みの設定に失敗しました！");
      cs.sendMessage(e.toString());
      return false;
    }
    if ("global".equals(arg[0])) {
      def = new Set(mc);
      saveCfg();
      cs.sendMessage("全体チャットを棒読みする設定にしました！");
      return true;
    } else if (arg[0].startsWith("c:")) {
      map.put(arg[0].substring(2), new Set(mc));
      saveCfg();
      cs.sendMessage(arg[0] + " のチャットを棒読みする設定にしました！");
      return true;
    } else if ("list".equals(arg[0])) {
      cs.sendMessage("全体チャットの読み上げ: " + Boolean.toString(def != null));
      cs.sendMessage("読み上げるチャンネル一覧-------------");
      for (final String s : map.keySet())
        cs.sendMessage(s);
      cs.sendMessage("-------------------------------");
      return true;
    }
    return false;
  }

  private static final class Q {
    final String c;
    final String s;
    final String p;

    Q(final String pc, final String ps, final String pp) {
      this.c = pc;
      this.s = ps;
      this.p = pp;
    }

  }

  public static void chat(final Player p, final String s, final String c) {
    if (Thread.State.NEW == I.getState()) {
      I.setDaemon(true);
      I.start();
    }
    q.offer(new Q(c, s, p.getDisplayName()));
  }

  private static InetSocketAddress a = null;
  static {
    try {
      a = new InetSocketAddress(InetAddress.getByAddress(new byte[] {127, 0, 0, 1}), 50001);
    } catch (final Exception e) {
      e.printStackTrace();
    }
  }
  private static final Queue<Q> q = new ConcurrentLinkedQueue<Q>();

  @Override
  public void run() {
    while (true) {
      if (q.isEmpty()) {
        try {
          Thread.sleep(50);
        } catch (final InterruptedException e) {
          e.printStackTrace();
        }
        continue;
      }
      try {
        final Q e = q.poll();
        final Set s = e.c == null ? def : map.get(e.c);
        if (s == null)
          continue;
        final byte[] r = (e.p + " " + e.s).getBytes(YogpstopTweaks.utf8);
        final Socket k = new Socket();
        k.connect(a);
        final OutputStream o = k.getOutputStream();
        o.write(0x01);// command
        o.write(0x00);
        o.write(0xFF);// speed
        o.write(0xFF);
        o.write(s.tone >>> 8 & 0xFF);// tone
        o.write(s.tone >>> 0 & 0xFF);
        o.write(0xFF);// volume
        o.write(0xFF);
        o.write(s.type >>> 8 & 0xFF);// type
        o.write(s.type >>> 0 & 0xFF);
        o.write(0x00);// charset
        o.write(r.length >> 24);
        o.write(r.length >> 16);
        o.write(r.length >> 8);
        o.write(r.length);
        o.write(r);
        o.flush();
        o.close();
        k.close();
      } catch (final IOException e) {
        ;
      }
    }
  }

}
