package org.yogpstop.yt;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.OutputStreamWriter;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.logging.Level;

import org.bukkit.configuration.file.FileConfiguration;

final class WebPost {
	private static String pw;
	private static URL url;

	static final void loadConfiguration(FileConfiguration f) {
		pw = f.getString("webpost.password");
		try {
			url = new URL(f.getString("webpost.url"));
		} catch (MalformedURLException e) {
			e.printStackTrace();
		}
	}

	static final void onEnable() {
		main(true);
	}

	static final void onDisable() {
		main(false);
	}

	private static final void main(boolean enable) {
		{
			try {
				HttpURLConnection http = (HttpURLConnection) url
						.openConnection();
				http.setRequestMethod("POST");
				http.setDoOutput(true);
				http.connect();
				OutputStreamWriter osw = new OutputStreamWriter(
						http.getOutputStream());
				osw.write("boot=" + (enable ? "1" : "0") + "&pw=" + pw);
				osw.flush();
				osw.close();
				BufferedInputStream bis = new BufferedInputStream(
						http.getInputStream());
				while (bis.read() != -1)
					;
				bis.close();
			} catch (IOException e1) {
				e1.printStackTrace();
			}
			YogpstopTweaks.logger.log(Level.INFO, "Post server packet.");
		}
	}
}
