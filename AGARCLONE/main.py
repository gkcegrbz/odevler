"""
Agar.io Tarzı Oyun - OOP Mimarisi
Nesne Tabanlı Programlama İlkeleriyle Geliştirilmiş Oyun
"""

import pygame
import math
import random
import sys
import os
from abc import ABC, abstractmethod

# ─────────────────────────────────────────────
#  ABSTRACTION & ENCAPSULATION: Temel Varlık
# ─────────────────────────────────────────────
class Entity(ABC):
    """Soyut temel sınıf - tüm oyun varlıkları buradan türer."""

    def __init__(self, x: float, y: float, radius: float, color: tuple):
        self._x = x
        self._y = y
        self._radius = radius
        self._color = color
        self._alive = True

    # ENCAPSULATION: Property'ler ile korumalı erişim
    @property
    def x(self): return self._x

    @property
    def y(self): return self._y

    @property
    def radius(self): return self._radius

    @property
    def color(self): return self._color

    @property
    def alive(self): return self._alive

    @alive.setter
    def alive(self, value: bool): self._alive = value

    def get_mass(self) -> float:
        return math.pi * self._radius ** 2

    def distance_to(self, other: 'Entity') -> float:
        return math.hypot(self._x - other._x, self._y - other._y)

    def overlaps(self, other: 'Entity') -> bool:
        return self.distance_to(other) < max(self._radius, other._radius)

    # ABSTRACTION: Alt sınıflar uygulamak zorunda
    @abstractmethod
    def update(self, dt: float): pass

    @abstractmethod
    def draw(self, surface: pygame.Surface, camera_x: float, camera_y: float): pass


# ─────────────────────────────────────────────
#  INHERITANCE: Yem sınıfı
# ─────────────────────────────────────────────
class Food(Entity):
    """Haritada rastgele dağılmış yem noktaları."""

    PULSE_SPEED = 2.0

    def __init__(self, x: float, y: float):
        color = random.choice([
            (255, 80,  80),  (80, 255,  80),  (80, 120, 255),
            (255, 200, 50),  (200, 50, 255),  (50, 230, 230),
            (255, 140, 0),   (180, 255, 100),
        ])
        super().__init__(x, y, random.uniform(5, 9), color)
        self._pulse_t = random.uniform(0, math.pi * 2)

    def update(self, dt: float):
        self._pulse_t += dt * self.PULSE_SPEED

    def draw(self, surface: pygame.Surface, camera_x: float, camera_y: float):
        if not self._alive:
            return
        sx = int(self._x - camera_x)
        sy = int(self._y - camera_y)
        pulse = 1.0 + 0.15 * math.sin(self._pulse_t)
        r = int(self._radius * pulse)
        pygame.draw.circle(surface, self._color, (sx, sy), r)
        highlight = tuple(min(255, c + 80) for c in self._color)
        pygame.draw.circle(surface, highlight, (sx - r // 4, sy - r // 4), max(2, r // 3))


# ─────────────────────────────────────────────
#  INHERITANCE: Blob temel sınıfı (oyuncu & AI ortak)
# ─────────────────────────────────────────────
class Blob(Entity):
    """Oyuncu ve AI için ortak blob davranışları."""

    GROWTH_RATE   = 0.30
    SHRINK_RATE   = 0.0008
    MIN_RADIUS    = 18.0
    MAX_RADIUS    = 260.0

    def __init__(self, x, y, radius, color, name=""):
        super().__init__(x, y, radius, color)
        self._name        = name
        self._score       = 0
        self._vel_x       = 0.0
        self._vel_y       = 0.0

    @property
    def name(self): return self._name

    @property
    def score(self): return self._score

    def _get_speed(self) -> float:
        """Büyüdükçe yavaşlama (fizik dengesi)."""
        return max(1.8, 9.0 - self._radius * 0.025)

    def eat_food(self, food: Food):
        if not food.alive:
            return
        if self.overlaps(food) and self._radius > food.radius * 0.8:
            food.alive = False
            growth = food.radius * self.GROWTH_RATE
            self._radius = min(self.MAX_RADIUS, self._radius + growth)
            self._score += int(food.radius)

    def eat_blob(self, other: 'Blob'):
        """Büyük blob küçüğü yer."""
        if not other.alive or not self.alive:
            return False
        if self._radius > other._radius * 1.10 and self.overlaps(other):
            self._radius = min(self.MAX_RADIUS,
                               math.sqrt(self._radius**2 + other._radius**2 * 0.9))
            self._score += int(other.get_mass() // 10)
            other.alive = False
            return True
        return False

    def _apply_world_bounds(self, world_w: int, world_h: int):
        self._x = max(self._radius, min(world_w - self._radius, self._x))
        self._y = max(self._radius, min(world_h - self._radius, self._y))

    def draw(self, surface: pygame.Surface, camera_x: float, camera_y: float):
        if not self._alive:
            return
        sx = int(self._x - camera_x)
        sy = int(self._y - camera_y)
        r  = int(self._radius)

        # Gölge
        shadow_surf = pygame.Surface((r * 2 + 20, r * 2 + 20), pygame.SRCALPHA)
        pygame.draw.circle(shadow_surf, (0, 0, 0, 60), (r + 10, r + 14), r)
        surface.blit(shadow_surf, (sx - r - 10, sy - r - 10))

        # Ana daire
        pygame.draw.circle(surface, self._color, (sx, sy), r)

        # Parlak kenar
        border_c = tuple(min(255, c + 50) for c in self._color)
        pygame.draw.circle(surface, border_c, (sx, sy), r, max(2, r // 12))

        # Işık yansıması
        hi_r = max(3, r // 3)
        hi_c = tuple(min(255, c + 100) for c in self._color)
        pygame.draw.circle(surface, hi_c, (sx - r // 4, sy - r // 4), hi_r)

        # İsim etiketi
        if self._name:
            font = pygame.font.SysFont("segoeui", max(10, min(22, r // 2)), bold=True)
            txt  = font.render(self._name, True, (255, 255, 255))
            txt_r = txt.get_rect(center=(sx, sy))
            shadow_txt = font.render(self._name, True, (0, 0, 0))
            surface.blit(shadow_txt, txt_r.move(1, 1))
            surface.blit(txt, txt_r)


# ─────────────────────────────────────────────
#  INHERITANCE: Oyuncu
# ─────────────────────────────────────────────
class Player(Blob):
    """Kullanıcı kontrolündeki blob."""

    def __init__(self, x, y, name="Oyuncu"):
        color = (100, 200, 255)
        super().__init__(x, y, 22.0, color, name)

    def handle_input(self, mx: int, my: int, screen_w: int, screen_h: int):
        """Fare yönüne göre hareket."""
        dx = mx - screen_w / 2
        dy = my - screen_h / 2
        dist = math.hypot(dx, dy)
        if dist > 5:
            speed = self._get_speed()
            factor = min(1.0, dist / 200)
            self._vel_x = (dx / dist) * speed * factor
            self._vel_y = (dy / dist) * speed * factor
        else:
            self._vel_x *= 0.85
            self._vel_y *= 0.85

    def update(self, dt: float, world_w=4000, world_h=4000):
        self._x += self._vel_x
        self._y += self._vel_y
        self._radius = max(self.MIN_RADIUS,
                           self._radius - self._radius * self.SHRINK_RATE)
        self._apply_world_bounds(world_w, world_h)


# ─────────────────────────────────────────────
#  INHERITANCE + POLYMORPHISM: AI Bot
# ─────────────────────────────────────────────
class AIBot(Blob):
    """
    POLYMORPHISM: update() davranışı Player'dan farklı —
    AI mantığı ile kendi kendine hareket eder.
    """

    STATES = ("wander", "chase_food", "chase_blob", "flee")

    def __init__(self, x, y, name, color=None):
        if color is None:
            color = (random.randint(100,255), random.randint(60,200), random.randint(60,200))
        super().__init__(x, y, random.uniform(18, 50), color, name)
        self._state      = "wander"
        self._target_x   = x + random.uniform(-300, 300)
        self._target_y   = y + random.uniform(-300, 300)
        self._think_timer = 0.0
        self._think_interval = random.uniform(0.8, 2.2)

    def _think(self, food_list: list, blob_list: list):
        """AI karar motoru."""
        nearby_food = [f for f in food_list if f.alive and self.distance_to(f) < 350]
        nearby_blobs= [b for b in blob_list if b.alive and b is not self]

        bigger   = [b for b in nearby_blobs if b.radius > self._radius * 1.15 and self.distance_to(b) < 400]
        smaller  = [b for b in nearby_blobs if b.radius < self._radius * 0.85 and self.distance_to(b) < 400]

        if bigger:
            closest_threat = min(bigger, key=lambda b: self.distance_to(b))
            dx = self._x - closest_threat.x
            dy = self._y - closest_threat.y
            self._target_x = self._x + dx * 3
            self._target_y = self._y + dy * 3
            self._state = "flee"
        elif smaller:
            prey = min(smaller, key=lambda b: self.distance_to(b))
            self._target_x = prey.x
            self._target_y = prey.y
            self._state = "chase_blob"
        elif nearby_food:
            closest = min(nearby_food, key=lambda f: self.distance_to(f))
            self._target_x = closest.x
            self._target_y = closest.y
            self._state = "chase_food"
        else:
            self._target_x = self._x + random.uniform(-400, 400)
            self._target_y = self._y + random.uniform(-400, 400)
            self._state = "wander"

    def update(self, dt: float, food_list=None, blob_list=None, world_w=4000, world_h=4000):
        """POLYMORPHISM: AI'ya özgü update."""
        self._think_timer += dt
        if self._think_timer >= self._think_interval:
            self._think_timer = 0
            if food_list and blob_list:
                self._think(food_list, blob_list)

        dx = self._target_x - self._x
        dy = self._target_y - self._y
        dist = math.hypot(dx, dy)
        if dist > 2:
            speed = self._get_speed() * (1.1 if self._state == "flee" else 0.9)
            self._x += (dx / dist) * speed
            self._y += (dy / dist) * speed

        self._radius = max(self.MIN_RADIUS,
                           self._radius - self._radius * self.SHRINK_RATE * 0.5)
        self._apply_world_bounds(world_w, world_h)


# ─────────────────────────────────────────────
#  ENCAPSULATION: Kamera sistemi
# ─────────────────────────────────────────────
class Camera:
    def __init__(self, screen_w: int, screen_h: int):
        self._x = 0.0
        self._y = 0.0
        self._screen_w = screen_w
        self._screen_h = screen_h
        self._smoothing = 0.08

    @property
    def x(self): return self._x

    @property
    def y(self): return self._y

    def follow(self, target: Entity):
        target_x = target.x - self._screen_w / 2
        target_y = target.y - self._screen_h / 2
        self._x += (target_x - self._x) * self._smoothing
        self._y += (target_y - self._y) * self._smoothing


# ─────────────────────────────────────────────
#  Leaderboard (Encapsulation örneği)
# ─────────────────────────────────────────────
class Leaderboard:
    def __init__(self):
        self._entries: list[tuple[str,int]] = []

    def update(self, blobs: list):
        self._entries = sorted(
            [(b.name, int(b.get_mass() // 100)) for b in blobs if b.alive],
            key=lambda e: e[1], reverse=True
        )[:10]

    def draw(self, surface: pygame.Surface):
        font_title = pygame.font.SysFont("segoeui", 18, bold=True)
        font_entry = pygame.font.SysFont("segoeui", 15)
        panel = pygame.Surface((190, 30 + len(self._entries) * 22), pygame.SRCALPHA)
        panel.fill((0, 0, 0, 140))
        surface.blit(panel, (surface.get_width() - 200, 10))

        title = font_title.render("🏆 Sıralama", True, (255, 220, 50))
        surface.blit(title, (surface.get_width() - 195, 14))

        for i, (name, score) in enumerate(self._entries):
            color = (255, 215, 0) if i == 0 else (200, 200, 200)
            txt = font_entry.render(f"{i+1}. {name}  {score}", True, color)
            surface.blit(txt, (surface.get_width() - 195, 36 + i * 22))


# ─────────────────────────────────────────────
#  ENCAPSULATION: Ana Oyun Sınıfı
# ─────────────────────────────────────────────
class Game:
    WORLD_W   = 4000
    WORLD_H   = 4000
    FOOD_COUNT= 600
    BOT_COUNT = 18
    BOT_NAMES = ["Kerem","Ayşe","Burak","Selin","Mert","Deniz","Ece","Tarık",
                 "Elif","Can","İrem","Uğur","Zeynep","Arda","Naz","Sercan","Berk","Şule"]

    def __init__(self, player_name: str = "Oyuncu"):
        pygame.init()
        info = pygame.display.Info()
        self._sw = min(1280, info.current_w)
        self._sh = min(720,  info.current_h)
        self._screen  = pygame.display.set_mode((self._sw, self._sh))
        pygame.display.set_caption("🟢 AgarClone — OOP Oyun Projesi")
        self._clock   = pygame.time.Clock()
        self._camera  = Camera(self._sw, self._sh)
        self._leaderboard = Leaderboard()
        self._running = True
        self._game_over = False
        self._spawn_player(player_name)
        self._spawn_food()
        self._spawn_bots()
        self._bg_surface = self._build_background()

    def _spawn_player(self, name):
        self._player = Player(self.WORLD_W // 2, self.WORLD_H // 2, name)

    def _spawn_food(self):
        self._foods: list[Food] = [
            Food(random.uniform(20, self.WORLD_W - 20),
                 random.uniform(20, self.WORLD_H - 20))
            for _ in range(self.FOOD_COUNT)
        ]

    def _spawn_bots(self):
        self._bots: list[AIBot] = []
        for i in range(self.BOT_COUNT):
            x = random.uniform(100, self.WORLD_W - 100)
            y = random.uniform(100, self.WORLD_H - 100)
            self._bots.append(AIBot(x, y, self.BOT_NAMES[i % len(self.BOT_NAMES)]))

    def _build_background(self) -> pygame.Surface:
        """Izgara arka plan (tek seferlik oluşturulur)."""
        surf = pygame.Surface((self._sw, self._sh))
        surf.fill((15, 20, 35))
        GRID = 60
        for x in range(0, self._sw, GRID):
            pygame.draw.line(surf, (25, 35, 55), (x, 0), (x, self._sh))
        for y in range(0, self._sh, GRID):
            pygame.draw.line(surf, (25, 35, 55), (0, y), (self._sw, y))
        return surf

    def _draw_background(self):
        """Kamera ofsetine göre ızgara kaydırma."""
        GRID = 60
        off_x = int(self._camera.x) % GRID
        off_y = int(self._camera.y) % GRID
        self._screen.fill((15, 20, 35))
        for x in range(-off_x, self._sw, GRID):
            pygame.draw.line(self._screen, (25, 35, 55), (x, 0), (x, self._sh))
        for y in range(-off_y, self._sh, GRID):
            pygame.draw.line(self._screen, (25, 35, 55), (0, y), (self._sw, y))

    def _draw_world_border(self):
        bx = int(-self._camera.x)
        by = int(-self._camera.y)
        bw = self.WORLD_W
        bh = self.WORLD_H
        pygame.draw.rect(self._screen, (60, 80, 120),
                         pygame.Rect(bx, by, bw, bh), 4)

    def _respawn_food(self):
        dead = [f for f in self._foods if not f.alive]
        for f in dead[:3]:
            f._x = random.uniform(20, self.WORLD_W - 20)
            f._y = random.uniform(20, self.WORLD_H - 20)
            f._alive = True

    def _respawn_bot(self, bot: AIBot):
        bot._x = random.uniform(100, self.WORLD_W - 100)
        bot._y = random.uniform(100, self.WORLD_H - 100)
        bot._radius = random.uniform(18, 35)
        bot._score  = 0
        bot._alive  = True

    def _draw_hud(self):
        font = pygame.font.SysFont("segoeui", 18, bold=True)
        mass  = int(self._player.get_mass() // 100)
        score = self._player.score
        alive_bots = sum(1 for b in self._bots if b.alive)

        texts = [
            (f"Kütle: {mass}",   (100, 220, 255)),
            (f"Skor:  {score}",  (255, 220,  80)),
            (f"Botlar: {alive_bots}/{self.BOT_COUNT}", (180,180,180)),
        ]
        panel = pygame.Surface((170, 80), pygame.SRCALPHA)
        panel.fill((0, 0, 0, 130))
        self._screen.blit(panel, (10, 10))
        for i, (t, c) in enumerate(texts):
            self._screen.blit(font.render(t, True, c), (16, 14 + i * 24))

    def _draw_minimap(self):
        MAP_W, MAP_H = 150, 150
        mx, my = self._sw - MAP_W - 10, self._sh - MAP_H - 10
        mini = pygame.Surface((MAP_W, MAP_H), pygame.SRCALPHA)
        mini.fill((0, 0, 0, 160))
        pygame.draw.rect(mini, (60, 80, 120), pygame.Rect(0, 0, MAP_W, MAP_H), 2)

        sx = int(self._player.x / self.WORLD_W * MAP_W)
        sy = int(self._player.y / self.WORLD_H * MAP_H)
        pygame.draw.circle(mini, (100, 220, 255), (sx, sy), 5)

        for bot in self._bots:
            if bot.alive:
                bx = int(bot.x / self.WORLD_W * MAP_W)
                by = int(bot.y / self.WORLD_H * MAP_H)
                pygame.draw.circle(mini, bot.color, (bx, by), 3)

        self._screen.blit(mini, (mx, my))

    def _draw_game_over(self):
        overlay = pygame.Surface((self._sw, self._sh), pygame.SRCALPHA)
        overlay.fill((0, 0, 0, 180))
        self._screen.blit(overlay, (0, 0))
        font_big  = pygame.font.SysFont("segoeui", 64, bold=True)
        font_small= pygame.font.SysFont("segoeui", 28)
        t1 = font_big.render("GAME OVER", True, (255, 80, 80))
        t2 = font_small.render(f"Skor: {self._player.score}  —  R ile yeniden başla, Q ile çık", True, (220, 220, 220))
        self._screen.blit(t1, t1.get_rect(center=(self._sw//2, self._sh//2 - 40)))
        self._screen.blit(t2, t2.get_rect(center=(self._sw//2, self._sh//2 + 40)))

    def _update(self, dt: float):
        mx, my = pygame.mouse.get_pos()
        self._player.handle_input(mx, my, self._sw, self._sh)
        self._player.update(dt, self.WORLD_W, self.WORLD_H)

        all_blobs = [self._player] + self._bots
        for bot in self._bots:
            if bot.alive:
                bot.update(dt, self._foods, all_blobs, self.WORLD_W, self.WORLD_H)

        for food in self._foods:
            food.update(dt)
            self._player.eat_food(food)
            for bot in self._bots:
                if bot.alive:
                    bot.eat_food(food)

        for bot in self._bots:
            if not bot.alive:
                self._respawn_bot(bot)
                continue
            if self._player.alive:
                self._player.eat_blob(bot)
                bot.eat_blob(self._player)

        for i, b1 in enumerate(self._bots):
            for b2 in self._bots[i+1:]:
                if b1.alive and b2.alive:
                    b1.eat_blob(b2)

        if not self._player.alive:
            self._game_over = True

        self._respawn_food()
        self._camera.follow(self._player)
        self._leaderboard.update(all_blobs)

    def _draw(self):
        self._draw_background()
        self._draw_world_border()

        cx, cy = self._camera.x, self._camera.y
        for food in self._foods:
            food.draw(self._screen, cx, cy)
        for bot in self._bots:
            bot.draw(self._screen, cx, cy)
        self._player.draw(self._screen, cx, cy)

        self._draw_hud()
        self._draw_minimap()
        self._leaderboard.draw(self._screen)

        if self._game_over:
            self._draw_game_over()

        pygame.display.flip()

    def run(self):
        while self._running:
            dt = self._clock.tick(60) / 1000.0

            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self._running = False
                elif event.type == pygame.KEYDOWN:
                    if event.key == pygame.K_q:
                        self._running = False
                    elif event.key == pygame.K_r and self._game_over:
                        self.__init__(self._player.name)
                    elif event.key == pygame.K_ESCAPE:
                        self._running = False

            if not self._game_over:
                self._update(dt)
            self._draw()

        pygame.quit()


# ─────────────────────────────────────────────
#  Entry Point
# ─────────────────────────────────────────────
def main():
    # İsim girişi için basit pygame penceresi
    pygame.init()
    screen = pygame.display.set_mode((500, 300))
    pygame.display.set_caption("Oyuncu Adı Gir")
    font_big  = pygame.font.SysFont("segoeui", 36, bold=True)
    font_small= pygame.font.SysFont("segoeui", 20)
    clock = pygame.time.Clock()
    name  = ""
    running = True
    while running:
        screen.fill((15, 20, 35))
        title = font_big.render("🟢 AgarClone", True, (100, 220, 255))
        screen.blit(title, title.get_rect(center=(250, 80)))
        prompt = font_small.render("Adını gir ve Enter'a bas:", True, (180, 180, 180))
        screen.blit(prompt, prompt.get_rect(center=(250, 145)))
        box_color = (60, 130, 200)
        pygame.draw.rect(screen, box_color, pygame.Rect(100, 170, 300, 44), border_radius=8)
        pygame.draw.rect(screen, (100, 180, 255), pygame.Rect(100, 170, 300, 44), 2, border_radius=8)
        name_surf = font_big.render(name + "▌", True, (255, 255, 255))
        screen.blit(name_surf, name_surf.get_rect(center=(250, 192)))
        hint = font_small.render("(boş bırakırsan 'Oyuncu' olur)", True, (100, 100, 100))
        screen.blit(hint, hint.get_rect(center=(250, 255)))
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit(); sys.exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_RETURN:
                    running = False
                elif event.key == pygame.K_BACKSPACE:
                    name = name[:-1]
                else:
                    if len(name) < 16:
                        name += event.unicode
        pygame.display.flip()
        clock.tick(60)

    pygame.quit()
    player_name = name.strip() or "Oyuncu"
    game = Game(player_name)
    game.run()


if __name__ == "__main__":
    main()
