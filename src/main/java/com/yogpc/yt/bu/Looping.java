package com.yogpc.yt.bu;

import java.io.File;

import org.bukkit.plugin.Plugin;

public class Looping extends Thread {
  private static Looping I = null;
  private volatile boolean stop = false;

  private Looping() {}

  public static final synchronized void make(final Plugin p) {
    Backup.B = new File(p.getDataFolder(), "backups");
    Backup.B.mkdirs();
    Backup.P = p;
    if (I != null)
      return;
    I = new Looping();
    I.setDaemon(true);
    I.start();
  }

  public static final synchronized void end() {
    if (I == null)
      return;
    I.stop = true;
    I.interrupt();
    while (I.isAlive())
      try {
        I.join();
      } catch (final InterruptedException e) {
      }
    I = null;
  }

  @Override
  public void run() {
    while (true)
      try {
        if (this.stop)
          break;
        Thread.sleep(1000 * 60 * 20);
        new Start().start();
      } catch (final InterruptedException e) {
      }
  }
}
