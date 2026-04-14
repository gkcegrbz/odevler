# Teknik Dokümantasyon — Tekil Kurulum Kontrol Mekanizması

**Proje:** AgarClone — OOP Tabanlı Oyun  
**Konu:** Tekil Kurulum Kontrol Mekanizması Tasarımı ve Uygulaması

---

## 1. Genel Bakış

Bu belgede, AgarClone projesinde uygulanan tekil kurulum kontrol mekanizması açıklanmaktadır. Mekanizmanın amacı, aynı kurulum paketinin bir sisteme birden fazla kez uygulanmasını engellemektir.

Sistem şu temel prensibi izler: **kurulum bir kez yapılır, kilit dosyası kalıcı olarak saklanır, sonraki kurulum girişimleri bu kilit tespit edilerek engellenir.**

---

## 2. Neden Tekil Kurulum Gereklidir?

Bir yazılımın aynı sisteme defalarca kurulması çeşitli sorunlara yol açabilir:

- Yapılandırma dosyalarının üzerine yazılması ve ayarların sıfırlanması
- Çakışan dosyalar nedeniyle uygulamanın bozulması
- Kullanıcı verilerinin yanlışlıkla silinmesi
- Kayıt defteri veya sistem dosyalarında tekrarlı girişler oluşması

Bu nedenle profesyonel yazılımlarda kurulum programları, daha önce kurulum yapılıp yapılmadığını kontrol eden bir mekanizma içerir. Bu projede söz konusu mekanizma **kilit dosyası (lock file) yöntemi** ile uygulanmıştır.

---

## 3. Mekanizmanın Çalışma Prensibi

### 3.1 Kilit Dosyası Yöntemi

Kilit dosyası yöntemi, yazılım dünyasında yaygın olarak kullanılan basit ve güvenilir bir tekniktir. Temel mantığı şudur:

1. Kurulum tamamlandığında, belirli bir konuma küçük bir metin dosyası yazılır.
2. Bir sonraki kurulum girişiminde, bu dosyanın varlığı kontrol edilir.
3. Dosya mevcutsa kurulum durdurulur; değilse kurulum devam eder.

### 3.2 Akış Diyagramı

```
installer.py çalıştırıldı
            │
            ▼
┌─────────────────────────┐
│  --force bayrağı var mı? │
└─────────────────────────┘
      │ EVET         │ HAYIR
      │              ▼
      │   ┌─────────────────────────────┐
      │   │  installed.lock dosyası     │
      │   │  mevcut mu?                 │
      │   └─────────────────────────────┘
      │         │ EVET        │ HAYIR
      │         ▼             │
      │   ┌──────────┐        │
      │   │  ENGELLE │        │
      │   │  Çıkış:1 │        │
      │   └──────────┘        │
      │                       │
      └───────────────────────┘
                  │
                  ▼
        Python versiyonu kontrol
                  │
                  ▼
        Bağımlılıkları kontrol / yükle
                  │
                  ▼
        Dosyaları hedef dizine kopyala
                  │
                  ▼
        Başlatıcı dosyası oluştur
                  │
                  ▼
        installed.lock dosyasını oluştur  ◄── Kilit burada yazılır
                  │
                  ▼
             KURULUM BAŞARILI
```

---

## 4. Teknik Uygulama

### 4.1 Kilit Dosyasının Konumu

Kilit dosyası, işletim sistemine göre farklı konumlarda saklanır:

| İşletim Sistemi | Konum |
|---|---|
| Windows | `%LOCALAPPDATA%\AgarClone\installed.lock` |
| Linux | `~/.local/share/AgarClone/installed.lock` |
| macOS | `~/.local/share/AgarClone/installed.lock` |

Windows örneği:
```
C:\Users\KullaniciAdi\AppData\Local\AgarClone\installed.lock
```

Bu konum tercih edilmesinin sebebi, kullanıcı başına ayrı bir alan sağlaması ve yönetici yetkisi gerektirmemesidir. Ayrıca kullanıcının kolayca erişip silemeyeceği değil, farkında olmadan görmezden gelebileceği bir konumdur.

### 4.2 Kilit Dosyasının İçeriği

Kilit dosyası düz metin formatında olup aşağıdaki bilgileri içerir:

```
installed_at = 2026-04-10 08:47:00
install_dir  = C:\Users\VICTUS\AppData\Local\AgarClone\app
platform     = Windows 11
python       = 3.13.5
version      = 1.0.0
```

Bu bilgiler hem denetim (audit) hem de destek amacıyla saklanır. Herhangi bir sorun yaşandığında kurulumun ne zaman ve nerede yapıldığı bu dosyadan anlaşılabilir.

### 4.3 InstallLock Sınıfı

Kilit mekanizması `InstallLock` sınıfı içinde kapsüllenmiştir (encapsulation). Sınıfın tüm metotları `@classmethod` olarak tanımlanmıştır; bu sayede nesne oluşturmadan doğrudan sınıf üzerinden çağrılabilir.

```python
class InstallLock:

    LOCK_DIR  = Path(...) / "AgarClone"
    LOCK_FILE = LOCK_DIR / "installed.lock"

    @classmethod
    def is_installed(cls) -> bool:
        # Kilit dosyası var mı kontrol et
        return cls.LOCK_FILE.exists()

    @classmethod
    def create(cls, install_dir: Path):
        # Kilit dosyasını oluştur ve meta bilgileri yaz
        cls.LOCK_DIR.mkdir(parents=True, exist_ok=True)
        cls.LOCK_FILE.write_text(...)

    @classmethod
    def remove(cls):
        # Kilidi kaldır (--uninstall komutu için)
        if cls.LOCK_FILE.exists():
            cls.LOCK_FILE.unlink()
```

### 4.4 Kontrol Mantığı

Installer'ın `run()` metodunda kilit kontrolü **ilk adım olarak** yapılır. Bu sayede gereksiz hiçbir işlem (Python kontrolü, pip, dosya kopyalama) çalıştırılmaz:

```python
def run(self) -> int:

    # ── Tekil Kurulum Kontrolü (ilk kontrol) ──
    if InstallLock.is_installed() and not self._force:
        print("KURULUM ZATEN YAPILMIŞ — ENGELLENDİ")
        return 1  # hata kodu ile çık

    # Buraya ancak kilit yoksa veya --force varsa gelinir
    # [1/4] Python kontrolü
    # [2/4] Bağımlılık kontrolü
    # [3/4] Dosya kopyalama
    # [4/4] Başlatıcı oluşturma
    InstallLock.create(install_dir)  # ← en sona kilit yaz
    return 0  # başarı
```

Kilit dosyasının kurulumun **en sonunda** yazılması önemlidir. Eğer kurulum sırasında bir hata oluşursa kilit yazılmamış olur ve kullanıcı sorunu giderdikten sonra kurulumu tekrar çalıştırabilir.

---

## 5. Kritik Özellik: Uygulama Silinse de Kilit Kalır

Bu mekanizmanın en önemli özelliği şudur: **Kullanıcı oyunu sistemden kaldırsa dahi kilit dosyası AppData konumunda varlığını sürdürür.**

Senaryo:

```
1. Kullanıcı installer.py'yi çalıştırır → kurulum yapılır → kilit oluşur
2. Kullanıcı oyun klasörünü siler
3. Kullanıcı installer.py'yi tekrar çalıştırır
4. Sistem kilit dosyasını bulur → kurulum ENGELLENİR
```

Bu davranış bilinçli bir tasarım kararıdır. Kurulum kilidinin uygulama dosyalarıyla aynı yerde tutulmaması, kilidi uygulamadan bağımsız kılar.

---

## 6. Komut Satırı Seçenekleri

| Komut | Açıklama |
|---|---|
| `python installer.py` | Normal kurulum — kilit varsa engeller |
| `python installer.py --force` | Kilidi yoksay, yeniden kur |
| `python installer.py --dir "C:\Hedef"` | Özel kurulum dizini belirt |
| `python installer.py --uninstall` | Sadece kilit dosyasını kaldır |

`--force` bayrağı yalnızca yetkili kullanıcılar veya geliştirici tarafından kullanılmak üzere tasarlanmıştır. Normal kullanıcıya sunulan arayüzde bu seçenek gösterilmez.

---

## 7. Hata Durumları ve Çıkış Kodları

| Durum | Çıkış Kodu | Açıklama |
|---|---|---|
| Kurulum başarılı | `0` | Her şey yolunda |
| Kilit mevcut | `1` | Kurulum engellendi |
| Python versiyonu yetersiz | `1` | 3.9+ gerekli |
| Bağımlılık yüklenemedi | `1` | pip hatası |
| Dosya kopyalama hatası | `1` | İzin sorunu veya disk dolu |

---

## 8. Güvenlik Değerlendirmesi

Bu mekanizma teknik bir engel sağlar, ancak kırılabilir. Kullanıcı aşağıdaki yollarla kilidi atlayabilir:

- Kilit dosyasını manuel olarak silmek
- `--force` bayrağını kullanmak
- Farklı bir kullanıcı hesabıyla kurulum yapmak

Bu kabul edilebilir bir durumdur. Mekanizmanın amacı kötü niyetli saldırılara karşı değil, **yanlışlıkla yapılan tekrar kurulumları önlemek** içindir. Kurumsal yazılımlarda bu tür mekanizmaların üzerine ek doğrulama katmanları (lisans sunucusu, donanım kimliği vb.) eklenir; ancak bu proje kapsamı için kilit dosyası yöntemi yeterli ve uygun bir çözümdür.

---

## 9. Özet

| Özellik | Uygulama |
|---|---|
| Yöntem | Kilit dosyası (lock file) |
| Kilit konumu | `%LOCALAPPDATA%\AgarClone\installed.lock` |
| Kontrol zamanı | Installer başlatıldığında, ilk adım olarak |
| Kilit yazma zamanı | Kurulum başarıyla tamamlandıktan sonra |
| Uygulama silinirse | Kilit kalır, tekrar kurulum engellenir |
| Geçersiz kılma | `--force` bayrağı ile mümkün |
| Sorumlu sınıf | `InstallLock` |
