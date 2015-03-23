package org.yogpstop.yt;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.channels.FileChannel;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.logging.Level;

import org.bukkit.Bukkit;
import org.bukkit.World;
import org.bukkit.command.CommandSender;
import org.bukkit.configuration.file.FileConfiguration;

import de.idyl.winzipaes.AesZipFileEncrypter;
import de.idyl.winzipaes.impl.AESEncrypter;
import de.idyl.winzipaes.impl.AESEncrypterJCA;

final class EncryptionBackup {
	private static final HashMap<File, Long> todir = new HashMap<File, Long>();
	private static final ArrayList<String> skipWorld = new ArrayList<String>();
	private static String pass;
	private static long interval;

	static final void loadConfiguration(File dir, FileConfiguration fc) {
		skipWorld.clear();
		interval = fc.getInt("backup.interval") * 60 * 20;
		pass = fc.getString("backup.password");
		skipWorld.addAll(fc.getStringList("backup.skipworld"));
		try {
			todir.clear();
			File f = new File(dir, "backup.cfg");
			if (!f.exists())
				f.createNewFile();
			BufferedReader b = new BufferedReader(new InputStreamReader(
					new FileInputStream(f)));
			String s;
			while ((s = b.readLine()) != null) {
				if (s.equals(""))
					continue;
				String[] c = s.trim().split("=");
				todir.put(new File(c[0]), Long.parseLong(c[1]));
			}
			b.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	private static int taskID = -1;

	static final void join() {
		if (taskID == -1) {
			taskID = Bukkit.getScheduler().scheduleSyncDelayedTask(
					YogpstopTweaks.plugin, backup, interval);
			YogpstopTweaks.logger.log(Level.INFO, "Backup is scheduled.");
		}
	}

	static final boolean command(String cmd, String[] args, CommandSender cs) {
		if (cmd.equals("backup")) {
			backup.run();
			return true;
		}
		return false;
	}

	private static final void saveall_off() {
		Bukkit.savePlayers();
		for (World world : Bukkit.getWorlds()) {
			world.save();
			world.setAutoSave(false);
		}
	}

	private static final void zipDir(File directory, File zip) {
		try {
			AESEncrypter aese = new AESEncrypterJCA();
			aese.init(pass, 256);
			AesZipFileEncrypter azfe = new AesZipFileEncrypter(zip, aese);
			zipDir(directory, azfe, "");
			azfe.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	private static final void zipDir(File zipDir, AesZipFileEncrypter azfe,
			String path) throws IOException {
		String[] dirList = zipDir.list();
		for (int i = 0; i < dirList.length; ++i) {
			File f = new File(zipDir, dirList[i]);
			if (f.isDirectory()) {
				zipDir(f, azfe, path.concat(f.getName()).concat(FILE_SEPARATOR));
				continue;
			}
			azfe.add(f, path.concat(f.getName()), pass);
		}

	}

	private static final void copyDirectory(File src, File dest)
			throws IOException {
		if (src.isFile())
			copyFile(src, dest);
		if (!dest.exists())
			dest.mkdir();
		for (File ff : src.listFiles()) {
			String name = ff.getName();
			File d2 = new File(dest, name);
			if (ff.isDirectory())
				copyDirectory(ff, d2);
			else
				copyFile(ff, d2);
		}
	}

	static private final void delete(File f) {
		if (f.exists() == false) {
			return;
		}

		if (f.isFile()) {
			f.delete();
		}

		if (f.isDirectory()) {
			File[] files = f.listFiles();
			for (int i = 0; i < files.length; i++) {
				delete(files[i]);
			}
			f.delete();
		}
	}

	private static final File encryption(ArrayList<File> file) {
		File dir = new File("backups");
		dir.mkdir();
		try {
			for (World world : Bukkit.getWorlds()) {
				if (!skipWorld.contains(world.getName().toLowerCase())) {
					copyDirectory(world.getWorldFolder(),
							new File(dir, world.getName()));
				}
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		File time = new File(Long.toString(System.currentTimeMillis() / 1000)
				+ ".zip");
		zipDir(dir, time);
		delete(dir);
		return time;
	}

	private static final File copyFiles(File file) {
		long size = 0;
		size += getSize(file);
		for (File cache : todir.keySet()) {
			if (!cache.exists())
				cache.mkdir();
			long sizea = size + getSize(cache);
			if (sizea > todir.get(cache)) {
				deleteOver(cache, sizea - todir.get(cache));
			}
			copyFile(file, new File(cache, file.getName()));
		}
		return file;
	}

	private static final void copyFile(File from, File to) {
		try {
			FileInputStream srcIS = new FileInputStream(from);
			FileChannel srcChannel = srcIS.getChannel();
			FileOutputStream destOS = new FileOutputStream(to);
			FileChannel destChannel = destOS.getChannel();
			srcChannel.transferTo(0, srcChannel.size(), destChannel);
			srcChannel.close();
			destChannel.close();
			srcIS.close();
			destOS.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	private static final void deleteOver(File file, long over) {
		long total = 0;
		ArrayList<File> deletelist = new ArrayList<File>();
		ArrayList<File> cache = getSortedFiles(file);
		for (File f : cache) {
			deletelist.add(f);
			total += f.length();
			if (total > over)
				break;
		}
		for (File delete : deletelist) {
			delete.delete();
		}
	}

	private static final long getSize(File file) {
		long size = 0;
		if (file.isDirectory()) {
			File files[] = file.listFiles();
			if (files != null) {
				for (int i = 0; i < files.length; i++) {
					size += getSize(files[i]);
				}
			}
		} else {
			size = file.length();
		}
		return size;
	}

	private static final ArrayList<File> getSortedFiles(File file) {
		ArrayList<File> files = new ArrayList<File>();
		if (file.isDirectory()) {
			File[] filesc = file.listFiles();
			for (File f : filesc) {
				files.addAll(getSortedFiles(f));
			}
		} else {
			files.add(file);
		}
		Collections.sort(files, cmp);
		return files;
	}

	private static final ArrayList<File> getFileList() {
		ArrayList<File> result = new ArrayList<File>();
		for (World w : Bukkit.getWorlds()) {
			if (!skipWorld.contains(w.getName().toLowerCase()))
				result.add(w.getWorldFolder());
		}
		return result;
	}

	private static final Runnable backupRun = new Runnable() {
		@Override
		public void run() {
			copyFiles(encryption(getFileList())).delete();
			try {
				Bukkit.getScheduler()
						.callSyncMethod(YogpstopTweaks.plugin, saveoff).get();
			} catch (InterruptedException | ExecutionException e) {
				e.printStackTrace();
			}
			YogpstopTweaks.logger.log(Level.INFO, "Backup is done.");
		}
	};
	private static final Comparator<File> cmp = new Comparator<File>() {
		@Override
		public int compare(File o1, File o2) {
			return (int) (o1.lastModified() - o2.lastModified());
		}
	};
	private static final Runnable backup = new Runnable() {
		@Override
		public void run() {
			if (Bukkit.getOnlinePlayers().length == 0)
				taskID = -1;
			else {
				taskID = Bukkit.getScheduler().scheduleSyncDelayedTask(
						YogpstopTweaks.plugin, this, interval);
				YogpstopTweaks.logger.log(Level.INFO, "Backup is scheduled.");
			}
			YogpstopTweaks.logger.log(Level.INFO, "Backup is started.");
			saveall_off();
			(new Thread(backupRun)).start();
		}
	};
	private static final Callable<Boolean> saveoff = new Callable<Boolean>() {
		@Override
		public Boolean call() throws Exception {
			for (World world : Bukkit.getWorlds()) {
				world.setAutoSave(true);
			}
			return true;
		}
	};

	private final static String FILE_SEPARATOR = System
			.getProperty("file.separator");
}