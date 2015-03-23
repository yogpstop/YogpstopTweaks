package org.yogpstop.yt;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.List;
import java.util.logging.Level;

import org.bukkit.Bukkit;
import org.bukkit.configuration.file.FileConfiguration;

final class AdvancedShutdown {
	private static int timer = -1;
	private static long interval;
	private static final ArrayList<TimeScope> times = new ArrayList<TimeScope>();
	private static final Calendar c = Calendar.getInstance();

	static final void onEnable() {
		quit();
	}

	static final void loadConfiguration(FileConfiguration f) {
		interval = f.getInt("shutdown.interval") * 60 * 20;
		List<String> s = f.getStringList("shutdown.times");
		for (String a : s) {
			a = a.trim();
			if (a.matches("([01][0-9]|2[0-3])[0-5][0-9];([01][0-9]|[2][0-3])[0-5][0-9]")) {
				times.clear();
				times.add(new TimeScope(Byte.parseByte(a.substring(0, 2)), Byte
						.parseByte(a.substring(2, 4)), Byte.parseByte(a
						.substring(5, 7)), Byte.parseByte(a.substring(7, 9))));
			} else {
				YogpstopTweaks.logger.log(Level.WARNING,
						"config.yml has illegral value.");
			}
		}
	}

	static final void join() {
		if (timer != -1) {
			Bukkit.getScheduler().cancelTask(timer);
			timer = -1;
			YogpstopTweaks.logger.log(Level.INFO, "ShutdownTimer is canceled.");
		}
	}

	static final void quit() {
		if (Bukkit.getServer().getOnlinePlayers().length <= 1 && timer == -1) {
			c.setTimeInMillis(System.currentTimeMillis());
			for (TimeScope t : times) {
				if (t.contains(c)) {
					timer = Bukkit.getScheduler().scheduleSyncDelayedTask(
							YogpstopTweaks.plugin, shutdown, interval);
					break;
				}
			}
			if (timer == -1) {
				ArrayList<Long> k = new ArrayList<Long>();
				for (TimeScope t : times) {
					k.add(t.diff(c));
				}
				Collections.sort(k);
				timer = Bukkit.getScheduler().scheduleSyncDelayedTask(
						YogpstopTweaks.plugin, shutdown, k.get(0));
			}
			if (timer == -1) {
				YogpstopTweaks.logger.log(Level.WARNING,
						"ShutdownTimer can't set.");
			} else {
				YogpstopTweaks.logger.log(Level.INFO,
						"ShutdownTimer is scheduled.");
			}
		}
	}

	private static final Runnable shutdown = new Runnable() {
		@Override
		public void run() {
			org.bukkit.Bukkit.getServer().shutdown();
		}
	};

	private static final class TimeScope {
		private final short from, to;

		public TimeScope(byte from_h, byte from_m, byte to_h, byte to_m) {
			from = (short) (from_h * 60 + from_m);
			short cache = (short) (to_h * 60 + to_m);
			if (from > cache)
				to = (short) (cache + 1440);
			else
				to = cache;
		}

		public final boolean contains(Calendar c) {
			short time = (short) (c.get(Calendar.HOUR_OF_DAY) * 60 + c
					.get(Calendar.MINUTE));
			if (from > time)
				time += 1440;
			if (time < to)
				return true;
			return false;
		}

		public final long diff(Calendar c) {
			short now = (short) (c.get(Calendar.HOUR_OF_DAY) * 60 + c
					.get(Calendar.MINUTE));
			short diff = (short) (from - now);
			if (diff < 0)
				diff += 1440;
			return diff * 60 * 20;
		}
	}
}
