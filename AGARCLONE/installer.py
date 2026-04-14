"""
AgarClone Kurulum Programı
TEKİL KURULUM KONTROL MEKANİZMASI:
  - Kurulum tamamlanınca %LOCALAPPDATA%\AgarClone\installed.lock dosyası oluşturulur.
  - Aynı paket yeniden çalıştırılırsa lock dosyası okunur ve kurulum engellenir.
  - Uygulama sisteme sonradan kaldırılsa dahi lock dosyası kalır → tekrar kurulum engellenir.
  - Yeniden kurulum için --force bayrağı veya lock dosyasının manuel silinmesi gerekir.
"""

import os
import sys
import shutil
import subprocess
import platform
import argparse
from pathlib import Path
from datetime import datetime


# ─────────────────────────────────────────────
#  ENCAPSULATION: Kurulum Kilidi
# ─────────────────────────────────────────────
class InstallLock:
    """
    Tekil kurulum mekanizması.
    Lock dosyası hem kurulumun yapıldığını hem de meta bilgilerini saklar.
    """

    LOCK_DIR  = Path(os.getenv("LOCALAPPDATA", Path.home() / ".local" / "share")) / "AgarClone"
    LOCK_FILE = LOCK_DIR / "installed.lock"

    @classmethod
    def is_installed(cls) -> bool:
        return cls.LOCK_FILE.exists()

    @classmethod
    def get_info(cls) -> dict:
        if not cls.is_installed():
            return {}
        try:
            data = {}
            for line in cls.LOCK_FILE.read_text(encoding="utf-8").splitlines():
                if "=" in line:
                    k, v = line.split("=", 1)
                    data[k.strip()] = v.strip()
            return data
        except Exception:
            return {}

    @classmethod
    def create(cls, install_dir: Path):
        cls.LOCK_DIR.mkdir(parents=True, exist_ok=True)
        info = (
            f"installed_at = {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n"
            f"install_dir  = {install_dir}\n"
            f"platform     = {platform.system()} {platform.release()}\n"
            f"python       = {sys.version.split()[0]}\n"
            f"version      = 1.0.0\n"
        )
        cls.LOCK_FILE.write_text(info, encoding="utf-8")
        print(f"[LOCK] Kurulum kilidi oluşturuldu: {cls.LOCK_FILE}")

    @classmethod
    def remove(cls):
        if cls.LOCK_FILE.exists():
            cls.LOCK_FILE.unlink()
            print(f"[LOCK] Kilit kaldırıldı: {cls.LOCK_FILE}")


# ─────────────────────────────────────────────
#  ENCAPSULATION: Sistem Kontrol
# ─────────────────────────────────────────────
class SystemCheck:
    MIN_PYTHON = (3, 9)
    REQUIRED_PACKAGES = ["pygame"]

    @classmethod
    def check_python(cls) -> bool:
        return sys.version_info >= cls.MIN_PYTHON

    @classmethod
    def check_packages(cls) -> list[str]:
        missing = []
        for pkg in cls.REQUIRED_PACKAGES:
            try:
                __import__(pkg)
            except ImportError:
                missing.append(pkg)
        return missing

    @classmethod
    def install_packages(cls, packages: list[str]) -> bool:
        print(f"[PKG] Bağımlılıklar yükleniyor: {', '.join(packages)}")
        result = subprocess.run(
            [sys.executable, "-m", "pip", "install"] + packages,
            capture_output=True, text=True
        )
        if result.returncode == 0:
            print("[PKG] Bağımlılıklar başarıyla yüklendi.")
            return True
        else:
            print(f"[HATA] pip hatası:\n{result.stderr}")
            return False


# ─────────────────────────────────────────────
#  ENCAPSULATION: Dosya Kopyalayıcı
# ─────────────────────────────────────────────
class FileInstaller:

    def __init__(self, source_dir: Path, target_dir: Path):
        self._source = source_dir
        self._target = target_dir

    def install(self) -> bool:
        try:
            if self._target.exists():
                shutil.rmtree(self._target)
            shutil.copytree(self._source, self._target)
            print(f"[KOPYALA] {self._source}  →  {self._target}")
            return True
        except Exception as e:
            print(f"[HATA] Dosya kopyalanamadı: {e}")
            return False


# ─────────────────────────────────────────────
#  ENCAPSULATION: Başlatıcı Oluşturucu
# ─────────────────────────────────────────────
class LauncherCreator:

    @staticmethod
    def create(install_dir: Path) -> Path | None:
        system = platform.system()
        if system == "Windows":
            return LauncherCreator._create_windows(install_dir)
        elif system in ("Linux", "Darwin"):
            return LauncherCreator._create_unix(install_dir)
        return None

    @staticmethod
    def _create_windows(install_dir: Path) -> Path:
        bat_path = install_dir / "AgarClone.bat"
        bat_path.write_text(
            f'@echo off\n'
            f'cd /d "{install_dir}"\n'
            f'"{sys.executable}" src\\main.py\n'
            f'pause\n',
            encoding="utf-8"
        )
        print(f"[LAUNCHER] Windows başlatıcı: {bat_path}")
        return bat_path

    @staticmethod
    def _create_unix(install_dir: Path) -> Path:
        sh_path = install_dir / "agarclon.sh"
        sh_path.write_text(
            f'#!/bin/bash\n'
            f'cd "{install_dir}"\n'
            f'"{sys.executable}" src/main.py\n',
            encoding="utf-8"
        )
        sh_path.chmod(0o755)
        print(f"[LAUNCHER] Unix başlatıcı: {sh_path}")
        return sh_path


# ─────────────────────────────────────────────
#  ANA KURULUM AKIŞI
# ─────────────────────────────────────────────
class Installer:
    """Tüm kurulum adımlarını orkestre eden ana sınıf."""

    DEFAULT_INSTALL = Path(os.getenv("LOCALAPPDATA", Path.home() / ".local" / "share")) / "AgarClone" / "app"

    def __init__(self, force: bool = False, install_dir: Path | None = None):
        self._force = force
        self._install_dir = install_dir or self.DEFAULT_INSTALL

    def run(self) -> int:
        self._print_banner()

        # ── Tekil Kurulum Kontrolü ──────────────────
        if InstallLock.is_installed() and not self._force:
            info = InstallLock.get_info()
            print("\n╔══════════════════════════════════════════╗")
            print("║  ⚠️  KURULUM ZATEN YAPILMIŞ — ENGELLENDİ  ║")
            print("╚══════════════════════════════════════════╝")
            print(f"  Kurulum tarihi : {info.get('installed_at','?')}")
            print(f"  Kurulum dizini : {info.get('install_dir','?')}")
            print(f"  Kilit dosyası  : {InstallLock.LOCK_FILE}")
            print("\n  Yeniden kurmak için:  python installer.py --force")
            return 1

        # ── Python Versiyonu ────────────────────────
        print("\n[1/4] Python versiyonu kontrol ediliyor...")
        if not SystemCheck.check_python():
            print(f"[HATA] Python {SystemCheck.MIN_PYTHON[0]}.{SystemCheck.MIN_PYTHON[1]}+ gerekli.")
            return 1
        print(f"  ✓ Python {sys.version.split()[0]}")

        # ── Bağımlılıklar ───────────────────────────
        print("\n[2/4] Bağımlılıklar kontrol ediliyor...")
        missing = SystemCheck.check_packages()
        if missing:
            if not SystemCheck.install_packages(missing):
                return 1
        else:
            print("  ✓ Tüm bağımlılıklar mevcut.")

        # ── Dosyaları Kopyala ───────────────────────
        print("\n[3/4] Oyun dosyaları kopyalanıyor...")
        source = Path(__file__).parent.parent
        file_installer = FileInstaller(source, self._install_dir)
        if not file_installer.install():
            return 1

        # ── Başlatıcı ───────────────────────────────
        print("\n[4/4] Başlatıcı oluşturuluyor...")
        launcher = LauncherCreator.create(self._install_dir)

        # ── Kilidi Oluştur ──────────────────────────
        InstallLock.create(self._install_dir)

        # ── Başarı ──────────────────────────────────
        print("\n╔══════════════════════════════════════╗")
        print("║  ✅  KURULUM BAŞARILI!                ║")
        print("╚══════════════════════════════════════╝")
        print(f"  Kurulum dizini : {self._install_dir}")
        if launcher:
            print(f"  Başlatıcı      : {launcher}")
        print("\n  Oyunu başlatmak için:")
        if platform.system() == "Windows":
            print(f'    {self._install_dir}\\AgarClone.bat')
        else:
            print(f'    {self._install_dir}/agarclon.sh')
        return 0

    def _print_banner(self):
        print("=" * 50)
        print("  AgarClone — Kurulum Programı v1.0")
        print("  Nesne Tabanlı Programlama Projesi")
        print("=" * 50)


def main():
    parser = argparse.ArgumentParser(description="AgarClone Kurulum Programı")
    parser.add_argument("--force", action="store_true",
                        help="Kurulum kilidi olsa bile yeniden kur")
    parser.add_argument("--dir", type=str, default=None,
                        help="Özel kurulum dizini")
    parser.add_argument("--uninstall", action="store_true",
                        help="Sadece kurulum kilidini kaldır")
    args = parser.parse_args()

    if args.uninstall:
        InstallLock.remove()
        return

    install_dir = Path(args.dir) if args.dir else None
    installer = Installer(force=args.force, install_dir=install_dir)
    sys.exit(installer.run())


if __name__ == "__main__":
    main()
