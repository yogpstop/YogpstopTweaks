package com.yogpc.yt.bu;

import java.util.LinkedList;
import java.util.Queue;
import java.util.UUID;

import org.bukkit.Bukkit;
import org.bukkit.World;
import org.bukkit.plugin.Plugin;

public class SaveTask implements Runnable {
  private static final SaveTask I = new SaveTask();

  private SaveTask() {}

  private volatile boolean done = false;
  private final Queue<UUID> q = new LinkedList<UUID>();
  private World w = null;

  public static final synchronized World get(final Plugin p) {
    Bukkit.getScheduler().runTask(p, SaveTask.I);
    while (!I.done)
      try {
        Thread.sleep(50);
      } catch (final InterruptedException e1) {
      }
    I.done = false;
    return I.w;
  }

  @Override
  public void run() {
    if (this.w == null && this.q.isEmpty())
      for (final World e : Bukkit.getWorlds())
        this.q.add(e.getUID());
    else if (this.w != null) {
      // TODO should keep previous state?
      this.w.setAutoSave(true);
      this.w = null;
    }
    do {
      final UUID u = this.q.poll();
      if (u == null)
        break;
      this.w = Bukkit.getWorld(u);
    } while (this.w == null);
    if (this.w != null) {
      this.w.setAutoSave(false);
      this.w.save();
    }
    this.done = true;
  }
}