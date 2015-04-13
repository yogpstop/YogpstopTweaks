package com.yogpc.yt.bu;

import org.bukkit.command.CommandSender;
import org.bukkit.plugin.Plugin;

public class Start extends Thread {
  @Override
  public void run() {
    synchronized (Backup.class) {
      Backup.I.start();
      while (Backup.I.isAlive())
        try {
          Backup.I.join();
        } catch (final InterruptedException e) {
        }
    }
  }

  public static boolean com(final Plugin p, final String m, final String[] arg,
      final CommandSender cs) {
    if (!m.equals("backup"))
      return false;
    if (arg.length == 0) {
      new Start().start();
      cs.sendMessage("Backup scheduled!");
      return true;
    } else if (arg.length == 1)
      if ("start".equals(arg[0])) {
        Looping.make(p);
        cs.sendMessage("Auto backup started!");
        return true;
      } else if ("stop".equals(arg[0])) {
        Looping.end();
        cs.sendMessage("Auto backup stopped!");
        return true;
      }
    return false;
  }
}
