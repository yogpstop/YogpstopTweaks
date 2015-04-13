package com.yogpc.yt.bu;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.zip.Deflater;
import java.util.zip.GZIPInputStream;
import java.util.zip.InflaterInputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipOutputStream;

import org.bukkit.World;
import org.bukkit.plugin.Plugin;

public final class Backup extends Thread {
  static Plugin P;
  static File B;
  static final Backup I = new Backup();

  private Backup() {}

  private static final byte[] dcmp(final byte[] in, final boolean gzip) throws IOException {
    final InputStream is = new ByteArrayInputStream(in);
    final InflaterInputStream gis = gzip ? new GZIPInputStream(is) : new InflaterInputStream(is);
    final ByteArrayOutputStream os = new ByteArrayOutputStream(4096);
    final byte[] buf = new byte[4096];
    int len;
    while ((len = gis.read(buf)) > -1)
      os.write(buf, 0, len);
    os.close();
    gis.close();
    is.close();
    return os.toByteArray();
  }

  private static final void write(final byte[] ts, final int tso, final byte[] in, final File out)
      throws IOException {
    final OutputStream os = new FileOutputStream(out);
    if (ts != null)
      os.write(ts, tso, 4);
    os.write(in);
    os.close();
  }

  private static final byte[] read(final File f) throws IOException {
    final FileInputStream is = new FileInputStream(f);
    final byte[] ret = new byte[(int) f.length()];
    int p = 0;
    while (p < ret.length)
      p += is.read(ret, p, ret.length - p);
    is.close();
    return ret;
  }

  private static final byte[] jar_entry(final InputStream in, final long size) throws IOException {
    byte[] data;
    if (size > 0) {
      data = new byte[(int) size];
      int offset = 0;
      do
        offset += in.read(data, offset, data.length - offset);
      while (offset < data.length);
    } else {
      final ByteArrayOutputStream dataout = new ByteArrayOutputStream();
      data = new byte[4096];
      int len;
      while ((len = in.read(data)) != -1)
        dataout.write(data, 0, len);
      data = dataout.toByteArray();
    }
    return data;
  }

  private static final void jar_dir(final ZipOutputStream out, final List<String> dir,
      final String entry) throws IOException {
    final int i = entry.lastIndexOf('/');
    if (i >= 0) {
      final String s = entry.substring(0, i + 1);
      if (dir.contains(s))
        return;
      dir.add(s);
      out.putNextEntry(new ZipEntry(s));
      out.closeEntry();
      jar_dir(out, dir, entry.substring(0, i));
    }
  }

  private static final void mcr(final byte[] in, final String pbase) throws IOException {
    if (in.length < 8192)
      throw new RuntimeException("Too small MCRegion file");
    new File(pbase).mkdir();
    int os, len, lpos;
    for (lpos = 0; lpos < 4096; lpos += 4) {
      os = ((in[lpos] & 255) << 16 | (in[lpos + 1] & 255) << 8 | in[lpos + 2] & 255) << 12;
      len = (in[lpos + 3] & 255) << 12;
      if (len <= 0 || in.length < os + len)
        continue;
      len =
          (in[os] & 255) << 24 | (in[os + 1] & 255) << 16 | (in[os + 2] & 255) << 8 | in[os + 3]
              & 255;
      byte[] tmp = new byte[len];
      System.arraycopy(in, os + 5, tmp, 0, len);
      tmp = dcmp(tmp, in[os + 4] == 1);
      write(in, lpos + 4096, tmp, new File(pbase, Integer.toHexString(lpos)));
    }
  }

  private static final void deepCopy(final File src, final String dst) throws IOException {
    if (src.isDirectory()) {
      new File(dst).mkdir();
      for (final File f : src.listFiles())
        deepCopy(f, dst + File.separatorChar + f.getName());
    } else {
      final String lname = src.getName().toLowerCase();
      final byte[] data = read(src);
      if (lname.contains(".mcr") || lname.contains(".mca"))
        mcr(data, dst);
      else if (lname.contains(".dat") && !lname.contains("idcounts.dat")
          && !lname.contains("uid.dat"))
        write(null, 0, dcmp(data, true), new File(dst + ".!gz"));
      else
        write(null, 0, data, new File(dst));
    }
  }

  private static final File latest(final File ddir) throws IOException {
    long biggest = -1;
    File f = null;
    for (final String s : B.list()) {
      final long l = Long.parseLong(s, 16);
      if (biggest < l) {
        biggest = l;
        f = new File(B, s);
      }
    }
    if (f == null)
      return null;
    final InputStream is = new FileInputStream(f);
    final ZipInputStream zis = new ZipInputStream(is);
    ZipEntry ze;
    while ((ze = zis.getNextEntry()) != null) {
      if (!ze.isDirectory()) {
        final File o = new File(ddir, ze.getName().replace('/', File.separatorChar));
        o.getParentFile().mkdirs();
        write(null, 0, jar_entry(zis, ze.getSize()), o);
      }
      zis.closeEntry();
    }
    zis.close();
    is.close();
    return f;
  }

  private static final void diff(final File full, final File diff) throws IOException {
    if (diff.isDirectory() && full.isDirectory())
      for (final File f : diff.listFiles())
        diff(new File(full, f.getName()), f);
    else if (diff.isFile() && full.isFile()) {
      final byte[] bdiff = read(diff);
      final byte[] bfull = read(full);
      if (Arrays.equals(bdiff, bfull))
        diff.delete();
    }
  }

  private static final void crec(final String path, final File in, final ZipOutputStream zos,
      final List<String> dirs) throws IOException {
    if (in.isDirectory()) {
      for (final File f : in.listFiles())
        crec(path == null ? f.getName() : path + "/" + f.getName(), f, zos, dirs);
      return;
    }
    jar_dir(zos, dirs, path);
    zos.putNextEntry(new ZipEntry(path));
    zos.write(read(in));
    zos.closeEntry();
  }

  private static final void compress(final File dir, final File zip) throws IOException {
    final OutputStream os = new FileOutputStream(zip);
    final ZipOutputStream zos = new ZipOutputStream(os);
    final List<String> dirs = new ArrayList<String>();
    zos.setLevel(Deflater.BEST_COMPRESSION);
    crec(null, dir, zos, dirs);
    zos.close();
  }

  private static final void rm(final File i) {
    if (i.isDirectory())
      for (final File j : i.listFiles())
        rm(j);
    i.delete();
  }

  @Override
  public void run() {
    final File z_cur = new File(B, Long.toHexString(System.currentTimeMillis()));
    while (true) {
      final World w = SaveTask.get(P);
      if (w == null)
        break;
      try {
        final File d_cur = File.createTempFile("YogTweaks", "");
        d_cur.delete();
        deepCopy(w.getWorldFolder(), d_cur.getCanonicalPath());
        final File d_prv = File.createTempFile("YogTweaks", "");
        d_prv.delete();
        final File z_prv = latest(d_prv);
        if (d_prv.isDirectory()) {
          diff(d_cur, d_prv);
          compress(d_prv, z_prv);
        }
        compress(d_cur, z_cur);
        rm(d_cur);
        rm(d_prv);
      } catch (final IOException e) {
        throw new RuntimeException(e);
      }
    }
  }
}
