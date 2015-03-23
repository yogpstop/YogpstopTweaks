package com.yogpc.yt.wi;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.bukkit.Bukkit;
import org.bukkit.GameMode;
import org.bukkit.Location;
import org.bukkit.World;
import org.bukkit.command.CommandSender;
import org.bukkit.configuration.ConfigurationSection;
import org.bukkit.configuration.file.FileConfiguration;
import org.bukkit.configuration.file.YamlConfiguration;
import org.bukkit.entity.Player;
import org.bukkit.inventory.Inventory;
import org.bukkit.inventory.ItemStack;
import org.bukkit.inventory.PlayerInventory;
import org.bukkit.potion.PotionEffect;
import org.bukkit.util.Vector;

public final class PlayerData {
  public static File bd;
  // Player inventory
  private final Map<Integer, ItemStack> inv_p = new HashMap<Integer, ItemStack>();
  // Armour inventory
  private final Map<Integer, ItemStack> inv_a = new HashMap<Integer, ItemStack>();
  // Enderchest inventory
  private final Map<Integer, ItemStack> inv_e = new HashMap<Integer, ItemStack>();
  private final int held;
  private final GameMode gm;
  private final float xp;
  private final int xpLevel;
  private final int xpTotal;
  private final double health;
  private final int satiety;
  private final float saturation;
  private final float exhaustion;
  private final Location bed;
  private final int fire;
  private final int air;
  private final List<PotionEffect> potion = new ArrayList<PotionEffect>();
  private final Vector velocity;
  private final Location location;
  private final float fallDist;

  private PlayerData(final ConfigurationSection s, final String dm, final double dh, final int da,
      final Location dl) {
    c2m(s, this.inv_p, "inv_p");
    c2m(s, this.inv_a, "inv_a");
    c2m(s, this.inv_e, "inv_e");
    this.held = s.getInt("held", 0);
    this.gm = GameMode.valueOf(s.getString("gm", dm));
    this.xp = (float) s.getDouble("xp", 0);
    this.xpLevel = s.getInt("xpLevel", 0);
    this.xpTotal = s.getInt("xpTotal", 0);
    this.health = s.getDouble("health", dh);
    this.satiety = s.getInt("satiety", 20);
    this.saturation = (float) s.getDouble("saturation", 5);
    this.exhaustion = (float) s.getDouble("exhaustion", 0);
    this.bed = (Location) s.get("bed", null);
    this.fire = s.getInt("fire", 0);
    this.air = s.getInt("air", da);
    final List<?> l = s.getList("potion", new ArrayList<Object>());
    for (final Object p : l)
      this.potion.add((PotionEffect) p);
    this.velocity = (Vector) s.get("velocity", new Vector(0, 0, 0));
    this.location = (Location) s.get("location", dl);
    this.fallDist = (float) s.getDouble("fallDist", 0);
  }

  private void write(final ConfigurationSection s) {
    m2c(this.inv_p, s, "inv_p");
    m2c(this.inv_a, s, "inv_a");
    m2c(this.inv_e, s, "inv_e");
    s.set("held", Integer.valueOf(this.held));
    s.set("gm", this.gm.toString());
    s.set("xp", Double.valueOf(this.xp));
    s.set("xpLevel", Integer.valueOf(this.xpLevel));
    s.set("xpTotal", Integer.valueOf(this.xpTotal));
    s.set("health", Double.valueOf(this.health));
    s.set("satiety", Integer.valueOf(this.satiety));
    s.set("saturation", Double.valueOf(this.saturation));
    s.set("exhaustion", Double.valueOf(this.exhaustion));
    if (this.bed != null)
      s.set("bed", this.bed);
    s.set("fire", Integer.valueOf(this.fire));
    s.set("air", Integer.valueOf(this.air));
    s.set("potion", this.potion);
    s.set("velocity", this.velocity);
    if (this.location != null)
      s.set("location", this.location);
    s.set("fallDist", Double.valueOf(this.fallDist));
  }

  private PlayerData(final Player p, final Location loc) {
    final PlayerInventory pi = p.getInventory();
    a2m(pi.getContents(), this.inv_p);
    a2m(pi.getArmorContents(), this.inv_a);
    this.held = pi.getHeldItemSlot();
    a2m(p.getEnderChest().getContents(), this.inv_e);
    this.gm = p.getGameMode();
    this.xp = p.getExp();
    this.xpLevel = p.getLevel();
    this.xpTotal = p.getTotalExperience();
    this.health = p.getHealth();
    this.satiety = p.getFoodLevel();
    this.saturation = p.getSaturation();
    this.exhaustion = p.getExhaustion();
    this.bed = p.getBedSpawnLocation() == null ? null : p.getBedSpawnLocation().clone();
    this.fire = p.getFireTicks();
    this.air = p.getRemainingAir();
    for (final PotionEffect e : p.getActivePotionEffects())
      this.potion.add(new PotionEffect(e.getType(), e.getDuration(), e.getAmplifier(), e
          .isAmbient(), e.hasParticles()));
    this.velocity = p.getVelocity();
    this.location = loc;
    this.fallDist = p.getFallDistance();
  }

  private void write(final Player p, final boolean loc) {
    if (loc) {
      if (this.location != null)
        p.teleport(this.location);
      p.setVelocity(this.velocity);
      p.setFallDistance(this.fallDist);
    }
    final PlayerInventory pi = p.getInventory();
    pi.setContents(m2a(this.inv_p, new ItemStack[pi.getSize()]));
    pi.setArmorContents(m2a(this.inv_a, new ItemStack[4]));
    pi.setHeldItemSlot(this.held);
    final Inventory ei = p.getEnderChest();
    ei.setContents(m2a(this.inv_e, new ItemStack[ei.getSize()]));
    p.setGameMode(this.gm);
    p.setExp(this.xp);
    p.setLevel(this.xpLevel);
    p.setTotalExperience(this.xpTotal);
    p.setHealth(this.health);
    p.setFoodLevel(this.satiety);
    p.setSaturation(this.saturation);
    p.setExhaustion(this.exhaustion);
    p.setBedSpawnLocation(this.bed);
    p.setFireTicks(this.fire);
    p.setRemainingAir(this.air);
    for (final PotionEffect e : p.getActivePotionEffects())
      p.removePotionEffect(e.getType());
    p.addPotionEffects(this.potion);
  }

  // Serialize helpers

  private static void a2m(final ItemStack[] isa, final Map<Integer, ItemStack> inv) {
    for (int i = 0; i < isa.length; i++) {
      final ItemStack is = isa[i];
      if (is == null)
        continue;
      inv.put(Integer.valueOf(i), is.clone());
    }
  }

  private static ItemStack[] m2a(final Map<Integer, ItemStack> inv, final ItemStack[] isa) {
    for (final Integer i : inv.keySet()) {
      if (i.intValue() >= isa.length)
        continue;
      isa[i.intValue()] = inv.get(i).clone();
    }
    return isa;
  }

  private static void c2m(final ConfigurationSection s, final Map<Integer, ItemStack> inv,
      final String se) {
    final ConfigurationSection cs = s.getConfigurationSection(se);
    if (cs == null)
      return;
    for (final Map.Entry<String, Object> e : cs.getValues(false).entrySet())
      inv.put(Integer.valueOf(e.getKey()), (ItemStack) e.getValue());
  }

  private static void m2c(final Map<Integer, ItemStack> inv, final ConfigurationSection s,
      final String se) {
    final ConfigurationSection cs = s.createSection(se);
    for (final Map.Entry<Integer, ItemStack> e : inv.entrySet())
      cs.set(e.getKey().toString(), e.getValue());
  }

  // file wrapper

  private static void saveFile(final Player p, final String g, final Location loc) {
    final File gb = new File(bd, g);
    if (!gb.isDirectory() && !gb.mkdirs())
      return;
    final File f = new File(gb, p.getUniqueId().toString());
    final FileConfiguration fc = new YamlConfiguration();
    if (f.isFile())
      try {
        fc.load(f);
      } catch (final Exception e) {
        e.printStackTrace();
      }
    new PlayerData(p, loc).write(fc);
    try {
      fc.save(f);
    } catch (final Exception e) {
      e.printStackTrace();
    }
  }

  private static void loadFile(final Player p, final String g, final Location loc) {
    final File f = new File(new File(bd, g), p.getUniqueId().toString());
    final FileConfiguration fc = new YamlConfiguration();
    if (f.isFile())
      try {
        fc.load(f);
      } catch (final Exception e) {
        e.printStackTrace();
      }
    new PlayerData(fc, WorldData.gamemode(g), p.getMaxHealth(), p.getMaximumAir(), loc).write(p,
        loc != null);
    if (!WorldData.groupOf(p.getWorld()).equals(g))
      Bukkit.getLogger().warning(p.getUniqueId().toString() + " may tereport to invalid world!!");
  }

  public static void handle(final Player p, final World fw) {
    final String fr = WorldData.groupOf(fw);
    final String to = WorldData.groupOf(p.getWorld());
    if (!fr.equals(to)) {
      saveFile(p, fr, null);
      loadFile(p, to, null);
    }
  }

  private static boolean handle(final Player p, final String g) {
    if (!WorldData.hasPermission(g, p)) {
      p.sendMessage("You don't have permission to go to that world group!!");
      return false;
    }
    final World w = WorldData.getMain(g, null);
    if (w == null) {
      p.sendMessage("Invalid group name or cannot detect spawn world of group!!");
      return false;
    }
    final String fr = WorldData.groupOf(p.getWorld());
    if (fr.equals(g)) {
      p.sendMessage("You are already in specific group!!");
      return false;
    }
    saveFile(p, fr, p.getLocation().clone());
    loadFile(p, g, w.getSpawnLocation());
    return true;
  }

  public static boolean com(final String m, final String[] arg, final CommandSender cs) {
    if (!m.equals("chwgrp"))
      return false;
    if (!(cs instanceof Player)) {
      cs.sendMessage("Please execute that command from in-game.");
      return false;
    }
    if (arg.length != 1) {
      cs.sendMessage("Invalid use of change world group command.");
      return false;
    }
    return handle((Player) cs, arg[0]);
  }
}
