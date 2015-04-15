package com.yogpc.yt.bu;

public final class Looping extends Thread {
  private static Looping I = null;
  private volatile boolean stop = false;

  private Looping() {}

  static final synchronized void make() {
    if (I != null)
      return;
    I = new Looping();
    I.setDaemon(true);
    I.start();
  }

  static final synchronized void end() {
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
  public final void run() {
    while (true)
      try {
        if (this.stop)
          break;
        Thread.sleep(1000 * 60 * 20);
        Backup.start();
      } catch (final InterruptedException e) {
      }
  }
}
