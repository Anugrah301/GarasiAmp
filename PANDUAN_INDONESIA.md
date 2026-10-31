# Cara Membuat dan Memasang GARASI AMP VST3 di Studio One

## Hal penting terlebih dahulu

Project ini adalah kode asli plugin audio (C++), bukan HTML dan bukan file VST3 yang sudah dikompilasi. Kode pemrosesan suara telah lulus pengujian otomatis di lingkungan ini, tetapi **binary Windows belum dibuat dan belum dites di Studio One**. Untuk memperoleh plugin yang dapat digunakan di Studio One, lakukan build Windows berikut.

## A. Cara paling mudah (punya laptop/PC Windows)

1. Unduh ZIP project `GarasiAmp-Source-v0.1.zip` dan **Extract All**.
2. Install **Visual Studio 2022 Community**. Saat instalasi centang **Desktop development with C++**, termasuk Windows SDK.
3. Install **Git for Windows** dan **CMake** (versi minimal 3.24). Pastikan keduanya bisa dipanggil dari Command Prompt.
4. Sambungkan internet untuk pertama kali, karena sistem akan mengambil library JUCE 8.0.16-security.
5. Di folder GarasiAmp, klik dua kali **`BUILD_WINDOWS.cmd`**.
6. Setelah berhasil, script menghasilkan file ZIP VST3 di `dist\GarasiAmp-Windows-x64-VST3.zip` dan memasang VST3 pada akun Windows aktif.
7. Tutup Studio One, lalu buka kembali dan cari **Garasi Amp** di browser Effects.

Untuk Studio One yang lebih lama daripada 5.5.1, atau jika plugin belum ditemukan, jalankan PowerShell **As Administrator** dan ketik:

```powershell
cd "LOKASI_FOLDER_GARASIAMP"
.\build-windows.ps1 -SystemWide
```

Plugin akan disalin ke `C:\Program Files\Common Files\VST3`.

## B. Cara tanpa Visual Studio (GitHub Actions)

Jika tidak ingin memasang developer tools di PC, unggah isi folder project ke repository GitHub, kemudian buka tab **Actions > Windows VST3 Build > Run workflow**. Setelah proses sukses, ambil ZIP VST3 hasil build dari Artifacts. Ekstrak folder `.vst3` dan copy ke `C:\Program Files\Common Files\VST3` (butuh hak administrator) atau ke `%LOCALAPPDATA%\Programs\Common\VST3` untuk Studio One 5.5.1+.

GitHub Actions ini belum pernah dijalankan dari lingkungan ChatGPT ini, jadi tetap periksa hasil build dan plugin di Studio One.

## C. Cara menggunakan di Studio One

1. Colok gitar ke port **Instrument / Hi-Z** audio interface. Gunakan driver **ASIO** di Windows untuk latensi rendah.
2. Buat **Audio Track (Mono)**, pilih input audio interface yang terhubung ke gitar.
3. Aktifkan **Input Monitoring** pada track (ikon speaker). Jika audio interface direct monitoring aktif, matikan jalur direct monitoring agar tidak mendengar suara DI bersamaan dengan suara plugin.
4. Buka browser **Effects**, lalu cari dan drag **Garasi Amp** ke track gitar.
5. Mulai dari preset **01 GLASS CLEAN**. Atur INPUT dan MASTER agar suara tidak pecah di input/output.
6. Pilih **CRUNCH** atau **LEAD** untuk gain lebih tinggi. Hidupkan DELAY/REVERB jika perlu.
7. Tombol **SAVE** / **LOAD** menyimpan tone sebagai file `.xml`. Studio One juga menyimpan parameter plugin secara otomatis di dalam Song.

Jika tidak muncul di Studio One: **Options > Locations > VST Plug-ins**, centang **Scan at Startup**, pilih **Reset Blocklist** bila tersedia, lalu restart Studio One. Pastikan bundle `.vst3` berada di folder yang benar.

## D. Tahapan pengembangan menuju versi profesional

| Tahap | Yang dibuat | Kondisi |
| --- | --- | --- |
| 0.1 | Amp Clean/Crunch/Lead, noise gate, boost, EQ, cabinet filter, delay, reverb, GUI, preset | Kode tersedia; tes DSP lulus; build VST3 Windows belum diverifikasi |
| 0.2 | Upload speaker cabinet IR WAV asli dan convolution | Belum |
| 0.3 | Oversampling 4x untuk distorsi, parameter smoothing, CPU optimization | Belum |
| 0.4 | Tuner gitar, tap/tempo sync delay, A/B preset, wah/chorus | Belum |
| 1.0 | Installer .exe, validasi VST3, test DAW menyeluruh, signing | Belum |

## E. Perhatian

- Plugin memproses suara gitar asli, bukan membuat not MIDI gitar.
- Output tergantung pickup gitar, level DI, pengaturan audio interface, dan monitor/speaker.
- Model kabinet pada versi ini **bukan** cabinet IR. Suara akan berbeda dari plugin komersial yang memakai model amp dan IR lebih lanjut.
- Saat uji coba pertama, turunkan volume speaker/headphone agar terhindar dari lonjakan suara.

## F. Khusus Visual Studio Community 2026 (versi 18)

File `BUILD_WINDOWS.cmd` dalam paket revisi ini sudah mendeteksi Visual Studio 2026 secara otomatis. **Jangan menggunakan ZIP lama yang masih memaksa generator Visual Studio 2022.**

1. Ekstrak paket terbaru ke folder pendek seperti `C:\GarasiAmp`.
2. Pastikan Visual Studio Installer telah memasang workload **Desktop development with C++**.
3. Klik dua kali `BUILD_WINDOWS.cmd`. Script akan mencari Visual Studio 2026, CMake, dan Git.
4. Jika muncul pesan **Compatible CMake not found**, pasang CMake **4.2 atau lebih baru** dari https://cmake.org/download/ dengan opsi **Add CMake to PATH**, lalu jalankan ulang. Visual Studio 2026 membutuhkan generator yang pertama kali hadir di CMake 4.2.
5. Jika muncul pesan **Git was not found**, pasang Git for Windows dari https://git-scm.com/downloads/win, lalu jalankan ulang.
6. Ketika tulisan **INSTALLED:** tampil, tutup dan buka kembali Studio One. Cari **Garasi Amp** pada daftar Effects.

Build pertama kali perlu koneksi internet untuk mengunduh JUCE dan dapat menghasilkan banyak baris log. Jika gagal, kirim screenshot bagian error paling bawah agar dapat diperbaiki secara spesifik.
