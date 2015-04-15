package com.yogpc.yt.bu;

import java.io.File;
import java.io.IOException;
import java.lang.ProcessBuilder.Redirect;

import org.bukkit.World;
import org.bukkit.command.CommandSender;
import org.bukkit.configuration.file.YamlConfiguration;
import org.bukkit.plugin.Plugin;

public final class Backup implements Runnable {
  private static Plugin P;
  private static File B;
  private static String exe;
  private static final Backup I = new Backup();

  private Backup() {}

  @Override
  public final void run() {
    synchronized (Backup.class) {
      while (true) {
        final World w = SaveTask.get(P);
        if (w == null)
          break;
        try {
          final File out = new File(B, w.getName());
          out.mkdirs();
          final Process p =
              new ProcessBuilder(exe, w.getWorldFolder().getPath(), out.getPath())
                  .redirectOutput(Redirect.INHERIT).redirectError(Redirect.INHERIT).start();
          while (p.isAlive())
            try {
              p.waitFor();
            } catch (final InterruptedException e) {
            }
        } catch (final IOException e) {
          e.printStackTrace();
        }
      }
    }
  }

  static final void start() {
    final Thread t = new Thread(I);
    t.setDaemon(false);
    t.start();
  }

  public static final boolean com(final String m, final String[] arg, final CommandSender cs) {
    if (!m.equals("backup"))
      return false;
    if (arg.length == 0) {
      start();
      cs.sendMessage("Backup scheduled!");
      return true;
    } else if (arg.length == 1)
      if ("start".equals(arg[0])) {
        Looping.make();
        cs.sendMessage("Auto backup started!");
        return true;
      } else if ("stop".equals(arg[0])) {
        Looping.end();
        cs.sendMessage("Auto backup stopped!");
        return true;
      }
    return false;
  }

  public static final void init(final Plugin p) {
    final File cfgf = new File(p.getDataFolder(), "bup.yml");
    final YamlConfiguration cfg = YamlConfiguration.loadConfiguration(cfgf);
    exe = new File(p.getDataFolder(), cfg.getString("backup_exe", "backup.exe")).getPath();
    B = new File(p.getDataFolder(), "backups");
    P = p;
    if (cfg.getBoolean("loop", true))
      Looping.make();
  }
}
