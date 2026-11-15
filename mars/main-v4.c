/*
 * MARS KOLONISI YASAM DESTEK SIMULATORU
 * Algoritma ve Programlama Dersi Projesi
 * Derleme (macOS / Linux): clang -std=c11 -Wall -Wextra main-v4.c -o mars_sim
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

/* === ANSI RENK VE EKRAN KONTROL MAKROLARI === */
#define EKRAN_TEMIZLE "\033[2J\033[H"
#define CLR_RESET     "\033[0m"
#define CLR_BOLD      "\033[1m"
#define CLR_RED       "\033[31m"
#define CLR_GREEN     "\033[32m"
#define CLR_YELLOW    "\033[33m"
#define CLR_CYAN      "\033[36m"
#define CLR_DIM       "\033[2m"

#define MAX_LOG 7
#define LOG_UZUNLUK 74
#define SNAKEYES_URL "https://www.youtube.com/watch?v=dQw4w9WgXcQ"

/* === OYUN DURUM YAPISI (STATE) === */
typedef struct {
    int saglik;          /* 0 - 100 */
    int enerji;          /* 0 - 100 */
    int oksijen;         /* 0 - 100 */
    int su;              /* Birim */
    int yedekParca;      /* Adet */
    int modulHasari;     /* 0 - 100 */
    int kilitMuhurlu;    /* 0: Acik, 1: Muhurlu */
    int gun;             /* 1 - 15 */
    int tur;             /* Gecerli komut sayaci */
    char zorluk;         /* 'K', 'N', 'Z' */
    int kesifYuksekEsik; /* Enerji & oksijen yuksekken kesif esigi */
    int kesifDusukEsik;  /* Enerji veya oksijen dusukken kesif esigi */
    int firtinaTurSayisi;/* Zorluga gore kum firtinasi tur sayisi */
    int arkaArkayaBir;   /* Easter Egg (Snake Eyes): Arka arkaya gelen 1 zari sayaci */
    char logGecmisi[MAX_LOG][128];
    int logSayisi;
} GameState;

/* === YARDIMCI FONKSIYONLAR === */

/* Tampon bellek temizleme (scanf sonrasi kalan karakterler icin) */
void tamponTemizle(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* Bos dongu ile satir sonuna kadar okur */
    }
}

/* Kullanicidan Enter bekler (adim adim olay akisi icin) */
void enterBekle(const char *mesaj) {
    printf("\n%s%s%s", CLR_YELLOW, mesaj, CLR_RESET);
    fflush(stdout);
    tamponTemizle();
}

/* TUI Olay ve Zar Gunlugune satir ekler */
void logEkle(GameState *s, const char *mesaj) {
    if (s->logSayisi < MAX_LOG) {
        strncpy(s->logGecmisi[s->logSayisi], mesaj, LOG_UZUNLUK);
        s->logGecmisi[s->logSayisi][LOG_UZUNLUK] = '\0';
        s->logSayisi++;
    } else {
        for (int i = 0; i < MAX_LOG - 1; i++) {
            strcpy(s->logGecmisi[i], s->logGecmisi[i + 1]);
        }
        strncpy(s->logGecmisi[MAX_LOG - 1], mesaj, LOG_UZUNLUK);
        s->logGecmisi[MAX_LOG - 1][LOG_UZUNLUK] = '\0';
    }
}

/* Easter Egg (Snake Eyes): Arka arkaya 2 kez 1 geldiginde YouTube baglantisini acar */
void snakeyesTetikle(GameState *s) {
    logEkle(s, "[EASTER EGG] Snake Eyes! Arka arkaya 2 kez 1 geldi...");
    int sonuc = 0;
#ifdef __APPLE__
    sonuc = system("open \"" SNAKEYES_URL "\" >/dev/null 2>&1 &");
#elif defined(_WIN32)
    sonuc = system("start \"\" \"" SNAKEYES_URL "\"");
#else
    sonuc = system("xdg-open \"" SNAKEYES_URL "\" >/dev/null 2>&1 &");
#endif
    (void)sonuc;
}

/* Merkezi Zar Fonksiyonu: Sadece rand() % 100 uretir ve ardisik 1 sayacini izler */
int zarAt(GameState *s) {
    int zar = rand() % 100;
    if (zar == 1) {
        s->arkaArkayaBir++;
        if (s->arkaArkayaBir >= 2) {
            snakeyesTetikle(s);
            s->arkaArkayaBir = 0;
        }
    } else {
        s->arkaArkayaBir = 0;
    }
    return zar;
}

/* Zorunlu Sinirlandirma: Degerlerin [0, 100] araliginda kalmasini IF ile saglar */
void sinirlandir(GameState *s) {
    if (s->saglik < 0) s->saglik = 0;
    if (s->saglik > 100) s->saglik = 100;

    if (s->enerji < 0) s->enerji = 0;
    if (s->enerji > 100) s->enerji = 100;

    if (s->oksijen < 0) s->oksijen = 0;
    if (s->oksijen > 100) s->oksijen = 100;

    if (s->modulHasari < 0) s->modulHasari = 0;
    if (s->modulHasari > 100) s->modulHasari = 100;

    if (s->su < 0) s->su = 0;
    if (s->yedekParca < 0) s->yedekParca = 0;
}

/* 0-100 arasi degerler icin 20 karakterlik renkli TUI cubugu cizer */
void cubukYazdir(int deger, int tersRenk) {
    int dolu = deger / 5;
    if (dolu < 0) dolu = 0;
    if (dolu > 20) dolu = 20;

    const char *renk = CLR_GREEN;
    if (!tersRenk) {
        if (deger < 30) renk = CLR_RED;
        else if (deger < 60) renk = CLR_YELLOW;
        else renk = CLR_GREEN;
    } else {
        /* Modul hasari icin yuksek deger kirmizi, dusuk deger yesildir */
        if (deger > 70) renk = CLR_RED;
        else if (deger > 30) renk = CLR_YELLOW;
        else renk = CLR_GREEN;
    }

    printf("%s[", renk);
    for (int i = 0; i < 20; i++) {
        if (i < dolu) printf("#");
        else printf("-");
    }
    printf("]%s", CLR_RESET);
}

/* Ana TUI Kontrol Panelini cizer (76 karakter ic genislik) */
void panelCiz(const GameState *s) {
    const char *zorlukAdi = "NORMAL";
    if (s->zorluk == 'K') zorlukAdi = "KOLAY";
    else if (s->zorluk == 'Z') zorlukAdi = "ZOR";

    printf("%s", EKRAN_TEMIZLE);
    printf("%s╔════════════════════════════════════════════════════════════════════════════╗%s\n", CLR_CYAN, CLR_RESET);
    printf("%s║%s %sMARS KOLONISI YASAM DESTEK SIMULATORU%s                       %sZorluk: %-6s%s %s║%s\n",
           CLR_CYAN, CLR_RESET, CLR_BOLD, CLR_RESET, CLR_YELLOW, zorlukAdi, CLR_RESET, CLR_CYAN, CLR_RESET);
    printf("%s╠════════════════════════════════════════════════════════════════════════════╣%s\n", CLR_CYAN, CLR_RESET);

    /* Gun, Tur ve Hava Kilidi Satiri (76 karakter ic genislik) */
    printf("%s║%s GUN: %s%2d / 15%s   |   TUR: %s%2d%s   |   HAVA KILIDI: ",
           CLR_CYAN, CLR_RESET, CLR_BOLD, s->gun, CLR_RESET, CLR_BOLD, s->tur, CLR_RESET);
    if (s->kilitMuhurlu == 1) {
        printf("%s%-29s%s", CLR_GREEN, "MUHURLU (GUVENLI)", CLR_RESET);
    } else {
        printf("%s%-29s%s", CLR_RED, "ACIK (KORUMASIZ)", CLR_RESET);
    }
    printf("%s║%s\n", CLR_CYAN, CLR_RESET);

    printf("%s╠════════════════════════════════════════════════════════════════════════════╣%s\n", CLR_CYAN, CLR_RESET);

    /* Yasamsal Metrikler (Tam 76 karakter ic genislik: 16 + 22 + 38 = 76) */
    printf("%s║%s SAGLIK       : ", CLR_CYAN, CLR_RESET);
    cubukYazdir(s->saglik, 0);
    printf(" %3d%%  | SU          : %3d birim      %s║%s\n", s->saglik, s->su, CLR_CYAN, CLR_RESET);

    printf("%s║%s ENERJI       : ", CLR_CYAN, CLR_RESET);
    cubukYazdir(s->enerji, 0);
    printf(" %3d%%  | YEDEK PARCA : %3d adet       %s║%s\n", s->enerji, s->yedekParca, CLR_CYAN, CLR_RESET);

    printf("%s║%s OKSIJEN      : ", CLR_CYAN, CLR_RESET);
    cubukYazdir(s->oksijen, 0);
    printf(" %3d%%  | YENI GUN    : %d tur kaldi    %s║%s\n", s->oksijen, 3 - (s->tur % 3), CLR_CYAN, CLR_RESET);

    printf("%s║%s MODUL HASARI : ", CLR_CYAN, CLR_RESET);
    cubukYazdir(s->modulHasari, 1);
    printf(" %3d%%  | HEDEF       : 15. Gun Sonu   %s║%s\n", s->modulHasari, CLR_CYAN, CLR_RESET);

    /* Olay ve Zar Gunlugu Kutusu */
    printf("%s╠════════════════════════════════════════════════════════════════════════════╣%s\n", CLR_CYAN, CLR_RESET);
    printf("%s║%s %sOLAY VE ZAR GUNLUGU%s                                                        %s║%s\n",
           CLR_CYAN, CLR_RESET, CLR_BOLD, CLR_RESET, CLR_CYAN, CLR_RESET);
    printf("%s╟────────────────────────────────────────────────────────────────────────────╢%s\n", CLR_CYAN, CLR_RESET);

    for (int i = 0; i < MAX_LOG; i++) {
        if (i < s->logSayisi) {
            printf("%s║%s %-74s %s║%s\n", CLR_CYAN, CLR_RESET, s->logGecmisi[i], CLR_CYAN, CLR_RESET);
        } else {
            printf("%s║%s %-74s %s║%s\n", CLR_CYAN, CLR_RESET, "", CLR_CYAN, CLR_RESET);
        }
    }

    /* Komut Menusu */
    printf("%s╠════════════════════════════════════════════════════════════════════════════╣%s\n", CLR_CYAN, CLR_RESET);
    printf("%s║%s [G] Kesif   [O] Oksijen   [T] Tamir   [D] Durum                            %s║%s\n", CLR_CYAN, CLR_RESET, CLR_CYAN, CLR_RESET);
    printf("%s║%s [U] Dinlen  [K] Firtina   [H] Hava Kilidi   [Q] Cikis                      %s║%s\n", CLR_CYAN, CLR_RESET, CLR_CYAN, CLR_RESET);
    printf("%s╚════════════════════════════════════════════════════════════════════════════╝%s\n", CLR_CYAN, CLR_RESET);
}

/* Oyun Sonu Rapor Ekranini cizer (76 karakter ic genislik) */
void bitisEkraniCiz(const GameState *s, const char *baslik, const char *sebep) {
    /* Zorunlu Puan Formulu */
    int puan = (s->gun * 100) + s->saglik + s->oksijen - s->modulHasari;

    printf("%s", EKRAN_TEMIZLE);
    printf("%s╔════════════════════════════════════════════════════════════════════════════╗%s\n", CLR_CYAN, CLR_RESET);
    printf("%s║%s %s%-74s%s %s║%s\n", CLR_CYAN, CLR_RESET, CLR_BOLD, baslik, CLR_RESET, CLR_CYAN, CLR_RESET);
    printf("%s╠════════════════════════════════════════════════════════════════════════════╣%s\n", CLR_CYAN, CLR_RESET);
    printf("%s║%s Sebep : %-66s %s║%s\n", CLR_CYAN, CLR_RESET, sebep, CLR_CYAN, CLR_RESET);
    printf("%s╟────────────────────────────────────────────────────────────────────────────╢%s\n", CLR_CYAN, CLR_RESET);
    printf("%s║%s Hayatta Kalinan Gun : %-52d %s║%s\n", CLR_CYAN, CLR_RESET, s->gun, CLR_CYAN, CLR_RESET);
    printf("%s║%s Toplam Gecerli Tur  : %-52d %s║%s\n", CLR_CYAN, CLR_RESET, s->tur, CLR_CYAN, CLR_RESET);
    printf("%s║%s Son Saglik          : %-52d %s║%s\n", CLR_CYAN, CLR_RESET, s->saglik, CLR_CYAN, CLR_RESET);
    printf("%s║%s Son Enerji          : %-52d %s║%s\n", CLR_CYAN, CLR_RESET, s->enerji, CLR_CYAN, CLR_RESET);
    printf("%s║%s Son Oksijen         : %-52d %s║%s\n", CLR_CYAN, CLR_RESET, s->oksijen, CLR_CYAN, CLR_RESET);
    printf("%s║%s Son Modul Hasari    : %-52d %s║%s\n", CLR_CYAN, CLR_RESET, s->modulHasari, CLR_CYAN, CLR_RESET);
    printf("%s║%s Kalan Su / Parca    : %-3d birim / %-3d adet                                 %s║%s\n",
           CLR_CYAN, CLR_RESET, s->su, s->yedekParca, CLR_CYAN, CLR_RESET);
    printf("%s╠════════════════════════════════════════════════════════════════════════════╣%s\n", CLR_CYAN, CLR_RESET);
    printf("%s║%s %sTOPLAM SIMULASYON PUANI: %-49d%s %s║%s\n", CLR_CYAN, CLR_RESET, CLR_YELLOW, puan, CLR_RESET, CLR_CYAN, CLR_RESET);
    printf("%s╚════════════════════════════════════════════════════════════════════════════╝%s\n", CLR_CYAN, CLR_RESET);
}

/* === KOMUT FONKSIYONLARI === */

/* [D] Durum Raporu: Kritik uyarilari ve && ile genel guvenlik durumunu kontrol eder (Tur harcamaz) */
int komutDurumRaporu(GameState *s) {
    logEkle(s, "--- [D] DETAYLI SISTEM DURUM RAPORU ---");

    if (s->oksijen < 25) {
        logEkle(s, "[KRITIK UYARI] Oksijen seviyesi kritik duzeyde (<25)!");
    }
    if (s->saglik < 30) {
        logEkle(s, "[KRITIK UYARI] Saglik durumu tehlikeli (<30)!");
    }
    if (s->modulHasari > 70) {
        logEkle(s, "[KRITIK UYARI] Modul hasari cok yuksek (>70)!");
    }
    if (s->enerji < 20) {
        logEkle(s, "[UYARI] Enerji rezervleri dusuk (<20)!");
    }
    if (s->su < 2) {
        logEkle(s, "[UYARI] Su stogu yetersiz (<2)!");
    }

    /* Tum degerler guvenli aralikta ise tek && kosulu ile bildirilir */
    if (s->saglik >= 30 && s->enerji >= 20 && s->oksijen >= 25 && s->modulHasari <= 70 && s->su >= 2) {
        logEkle(s, "[DURUM] TUM SISTEMLER NORMAL");
    }

    return 0; /* Tur sayacini artirmaz */
}

/* [U] Uyu / Dinlen: Ic ice IF ile oksijen kontrolu yaparak enerji ve saglik yeniler */
int komutDinlen(GameState *s) {
    s->enerji += 25;
    s->oksijen -= 6;

    /* Ic ice IF yapisi: Oksijen < 20 ise saglik artmaz, azalir (negatif degerler dahil) */
    if (s->oksijen < 20) {
        if (s->oksijen < 10) {
            s->saglik -= 15;
            logEkle(s, "[U] Dinlenme: Kritik oksijen (<10)! Hipoksi nedeniyle Saglik -15");
        } else {
            s->saglik -= 10;
            logEkle(s, "[U] Dinlenme: Oksijen <20 oldugu icin hipoksi! Enerji +25, Saglik -10");
        }
    } else {
        s->saglik += 15;
        logEkle(s, "[U] Dinlenme basarili: Enerji +25, Saglik +15, Oksijen -6");
    }

    sinirlandir(s);
    return 1;
}

/* [O] Oksijen Uret: Mod (%) operatoru ile 2 su -> 3 oksijen donusumu yapar */
int komutOksijenUret(GameState *s) {
    if (s->enerji < 10) {
        logEkle(s, "[IPTAL] Oksijen uretimi icin yetersiz enerji! (Gereken: 10)");
        return 0;
    }
    if (s->su < 2) {
        logEkle(s, "[IPTAL] Oksijen uretimi icin en az 2 birim su gerekiyor!");
        return 0;
    }
    if (s->oksijen >= 100) {
        logEkle(s, "[IPTAL] Oksijen tanklari zaten %100 dolu!");
        return 0;
    }

    int miktar = 0;
    printf("\nOksijen uretimi icin kullanilacak su miktari (Mevcut: %d): ", s->su);
    if (scanf("%d", &miktar) != 1) {
        tamponTemizle();
        logEkle(s, "[HATA] Gecersiz sayi girisi! Islem iptal edildi.");
        return 0;
    }
    tamponTemizle();

    if (miktar < 2 || miktar > s->su) {
        logEkle(s, "[IPTAL] Gecersiz su miktari (En az 2, en fazla mevcut su olmali).");
        return 0;
    }

    /* Mod (%) operatoru ile tek/cift su kontrolu */
    int artikSu = miktar % 2;
    int islenenSu = miktar - artikSu;
    int uretilenOksijen = (islenenSu / 2) * 3;

    s->su -= islenenSu;
    s->enerji -= 10;
    s->oksijen += uretilenOksijen;
    sinirlandir(s);

    char mesaj[128];
    if (artikSu == 1) {
        snprintf(mesaj, sizeof(mesaj), "[O] %d su islendi -> +%d Oksijen (1 birim tek su depoda kaldi).",
                 islenenSu, uretilenOksijen);
    } else {
        snprintf(mesaj, sizeof(mesaj), "[O] %d su islendi -> +%d Oksijen uretildi (Enerji -10).",
                 islenenSu, uretilenOksijen);
    }
    logEkle(s, mesaj);
    return 1;
}

/* [G] Kesif Gorevi: Mantiksal operatorler (&&, ||) ve zar/esik ciktisi */
int komutKesif(GameState *s) {
    if (s->enerji < 15) {
        logEkle(s, "[IPTAL] Kesif gorevi icin yetersiz enerji! (Gereken: 15)");
        return 0;
    }

    /* && operatoru ile enerji ve oksijen durumuna gore basari esigi belirleme */
    int esik = (s->enerji > 40 && s->oksijen > 30) ? s->kesifYuksekEsik : s->kesifDusukEsik;

    s->enerji -= 15;
    s->oksijen -= 10;

    int zar = zarAt(s);
    char zarMesaji[128];
    snprintf(zarMesaji, sizeof(zarMesaji), "[G] Kesif -> Zar: %d / Esik: %d -> %s",
             zar, esik, (zar < esik) ? "BASARILI" : "BASARISIZ");
    logEkle(s, zarMesaji);

    /* En az uc farkli kesif sonucu (miktarlar da rand() % 100 tabanli zardan turetilir) */
    if (zar < esik && zar < (esik / 2)) {
        int bulunanSu = 2 + (zar % 3); /* 2-4 birim su */
        s->su += bulunanSu;
        char sonuc[128];
        snprintf(sonuc, sizeof(sonuc), "    -> Yeralti buz tabakasi bulundu! +%d birim Su.", bulunanSu);
        logEkle(s, sonuc);
    } else if (zar < esik) {
        int bulunanParca = 1 + (zar % 2); /* 1-2 yedek parca */
        s->yedekParca += bulunanParca;
        char sonuc[128];
        snprintf(sonuc, sizeof(sonuc), "    -> Eski uydu enkazi bulundu! +%d Yedek Parca.", bulunanParca);
        logEkle(s, sonuc);
    } else {
        logEkle(s, "    -> Bolge tarandi ancak kaynak bulunamadi (Eli bos donuldu).");
    }

    /* || operatoru ile yaralanma kontrolu */
    if (zar < 10 || s->modulHasari > 70) {
        s->saglik -= 12;
        logEkle(s, "    -> [TEHLIKE] Kaza/ariza nedeniyle yaralandiniz! Saglik -12");
    }

    sinirlandir(s);
    return 1;
}

/* [T] Tamir: IF - ELSE IF - ELSE zinciri ile 3 kademeli hasar onarimi */
int komutTamir(GameState *s) {
    if (s->enerji < 10) {
        logEkle(s, "[IPTAL] Tamir islemi icin yetersiz enerji! (Gereken: 10)");
        return 0;
    }
    if (s->yedekParca <= 0) {
        logEkle(s, "[IPTAL] Hic yedek parca yok, tamir yapilamaz!");
        return 0;
    }
    if (s->modulHasari == 0) {
        logEkle(s, "[BILGI] Modul hasari zaten 0, tamire gerek yok.");
        return 0;
    }

    int gerekenParca = 0;
    int onarimMiktari = 0;
    const char *seviyeAdi = "";

    /* 3 kademeli IF - ELSE IF - ELSE hasar degerlendirmesi */
    if (s->modulHasari <= 30) {
        seviyeAdi = "Hafif Hasar";
        gerekenParca = 1;
        onarimMiktari = 15;
    } else if (s->modulHasari <= 70) {
        seviyeAdi = "Orta Hasar";
        gerekenParca = 2;
        onarimMiktari = 30;
    } else {
        seviyeAdi = "Agir Hasar";
        gerekenParca = 3;
        onarimMiktari = 45;
    }

    if (s->yedekParca < gerekenParca) {
        char uyari[128];
        snprintf(uyari, sizeof(uyari), "[IPTAL] %s onarimi icin %d parca gerekli (Mevcut: %d).",
                 seviyeAdi, gerekenParca, s->yedekParca);
        logEkle(s, uyari);
        return 0;
    }

    s->yedekParca -= gerekenParca;
    s->enerji -= 10;
    s->modulHasari -= onarimMiktari;
    sinirlandir(s);

    char mesaj[128];
    snprintf(mesaj, sizeof(mesaj), "[T] %s onarildi (-%d Parca, -10 Enerji) -> Hasar -%d",
             seviyeAdi, gerekenParca, onarimMiktari);
    logEkle(s, mesaj);
    return 1;
}

/* [K] Kum Firtinasi: FOR dongusu, mod (%) cift tur hasari, continue ve break */
int komutKumFirtinasi(GameState *s) {
    int hasarAlinanTur = 0;
    int toplamSaglikKaybi = 0;
    int toplamOksijenKaybi = 0;
    int esik = 50; /* Zar < 50 ise firtinadan korunma basarili */

    logEkle(s, "[K] KUM FIRTINASI BASLADI! Adim adim simule ediliyor...");

    for (int i = 1; i <= s->firtinaTurSayisi; i++) {
        panelCiz(s);
        int carpan = (i % 2 == 0) ? 2 : 1; /* Cift numarali turlarda hasar 2 katina cikar */
        int zar = zarAt(s);
        int basarili = (zar < esik);

        printf("\n%s--- FIRTINA TURU %d / %d (Ruzgar Carpani: %dx) ---%s\n",
               CLR_YELLOW, i, s->firtinaTurSayisi, carpan, CLR_RESET);
        printf("Zar: %d / Esik: %d -> %s\n",
               zar, esik, basarili ? "BASARILI (Korunma)" : "BASARISIZ (Etki Alindi)");

        if (basarili) {
            printf("Siginak dayanikli kaldi, bu turda hasar alinmadi.\n");
        } else {
            int olay = zar % 3; /* 0: Saglik kaybi, 1: Oksijen sizintisi, 2: Modul hasari */

            if (olay == 0) {
                hasarAlinanTur++;
                int kayip = 5 * carpan;
                s->saglik -= kayip;
                toplamSaglikKaybi += kayip;
                printf("%sUcusan enkaz yaralanmaya yol acti! Saglik -%d%s\n", CLR_RED, kayip, CLR_RESET);
            } else if (olay == 1) {
                /* Hava kilidi muhurluyse oksijen sizintisi continue ile atlanir */
                if (s->kilitMuhurlu == 1) {
                    printf("%sHava kilidi MUHURLU! Oksijen sizintisi engellendi (continue).%s\n",
                           CLR_GREEN, CLR_RESET);
                    sinirlandir(s);
                    if (i < s->firtinaTurSayisi) {
                        enterBekle("Sonraki firtina turu icin Enter'a basin...");
                    } else {
                        enterBekle("Firtina ozetini gormek icin Enter'a basin...");
                    }
                    continue;
                }
                hasarAlinanTur++;
                int kayip = 6 * carpan;
                s->oksijen -= kayip;
                toplamOksijenKaybi += kayip;
                printf("%sHava kilidi acik! Oksijen sizintisi yasandi: Oksijen -%d%s\n",
                       CLR_RED, kayip, CLR_RESET);
            } else {
                hasarAlinanTur++;
                int hasar = 7 * carpan;
                s->modulHasari += hasar;
                printf("%sDis paneller darbe aldi! Modul Hasari +%d%s\n", CLR_RED, hasar, CLR_RESET);
            }
        }

        sinirlandir(s);

        /* Saglik veya oksijen 0'a duserse dongu break ile derhal sonlandirilir */
        if (s->saglik <= 0 || s->oksijen <= 0) {
            printf("\n%s[KRITIK] Yasam belirtileri tukendi! Firtina dongusu kesiliyor (break).%s\n",
                   CLR_RED, CLR_RESET);
            enterBekle("Devam etmek icin Enter'a basin...");
            break;
        }

        if (i < s->firtinaTurSayisi) {
            enterBekle("Sonraki firtina turu icin Enter'a basin...");
        } else {
            enterBekle("Firtina ozetini gormek icin Enter'a basin...");
        }
    }

    /* Firtina sonrasi hava kilidi basinc nedeniyle tekrar acilir */
    s->kilitMuhurlu = 0;

    char ozet[128];
    snprintf(ozet, sizeof(ozet), "[K] Firtina Bitti | Hasarli Tur: %d | Kayip -> Saglik: -%d, Oksijen: -%d",
             hasarAlinanTur, toplamSaglikKaybi, toplamOksijenKaybi);
    logEkle(s, ozet);
    return 1;
}

/* [H] Hava Kilidi Sifresi: DO-WHILE dongusu ile 3 hakli dinamik sifre cozme */
int komutHavaKilidi(GameState *s) {
    if (s->kilitMuhurlu == 1) {
        logEkle(s, "[BILGI] Hava kilidi zaten muhurlu durumda.");
        return 0;
    }
    if (s->enerji < 15) {
        logEkle(s, "[IPTAL] Hava kilidi paneli icin en az 15 enerji gerekli!");
        return 0;
    }

    /* Sifre o anki gun sayisindan aritmetik olarak turetilir */
    char sifre = (char)('A' + (s->gun % 26));
    int kalanHak = 3;
    char tahmin = '\0';

    printf("\n%s=== HAVA KILIDI GUVENLIK TERMINALI ===%s\n", CLR_CYAN, CLR_RESET);
    printf("Ipucu: Sifre, Ingiliz alfabesinde 'A' harfinden bugunun gun sayisi (%d) kadar ilerideki harftir.\n", s->gun);

    /* Tek mantiksal ifadede hem sifre yanlisligini hem kalan hakki kontrol eden DO-WHILE */
    do {
        printf("\nKalan Hak: %d | Tek karakterlik sifre tahmininiz (A-Z): ", kalanHak);
        if (scanf(" %c", &tahmin) != 1) {
            tamponTemizle();
            continue;
        }
        tamponTemizle();
        tahmin = (char)toupper((unsigned char)tahmin);

        if (tahmin == sifre) {
            s->kilitMuhurlu = 1;
            logEkle(s, "[H] Sifre DOGRU! Hava kilidi basariyla MUHURLENDI.");
            return 1;
        }

        kalanHak--;
        s->enerji -= 5;
        sinirlandir(s);

        if (tahmin < sifre) {
            printf("%sYanlis sifre! Ipucu: Ileri git (Enerji -5)%s\n", CLR_YELLOW, CLR_RESET);
        } else {
            printf("%sYanlis sifre! Ipucu: Geri git (Enerji -5)%s\n", CLR_YELLOW, CLR_RESET);
        }

    } while (tahmin != sifre && kalanHak > 0);

    /* 3 deneme de basarisiz olursa modul hasari 20 artar */
    s->modulHasari += 20;
    sinirlandir(s);
    logEkle(s, "[H] 3 deneme basarisiz! Guvenlik kilitlendi, Modul Hasari +20.");
    return 1;
}

/* === ANA PROGRAM DONGUSU === */
int main(void) {
    srand((unsigned int)time(NULL));

    GameState oyun;
    memset(&oyun, 0, sizeof(GameState));
    oyun.gun = 1;
    oyun.tur = 0;
    oyun.kilitMuhurlu = 0;
    oyun.arkaArkayaBir = 0;

    /* 1. Zorluk Seviyesi Secimi (Ayri SWITCH-CASE ve gecersiz giris kontrolu) */
    char secim = '\0';
    int gecerliZorluk = 0;

    printf("%s", EKRAN_TEMIZLE);
    printf("%s╔════════════════════════════════════════════════════════════════════════════╗%s\n", CLR_CYAN, CLR_RESET);
    printf("%s║%s %sMARS KOLONISI YASAM DESTEK SIMULATORU - GOREV BASLANGICI%s                   %s║%s\n",
           CLR_CYAN, CLR_RESET, CLR_BOLD, CLR_RESET, CLR_CYAN, CLR_RESET);
    printf("%s╠════════════════════════════════════════════════════════════════════════════╣%s\n", CLR_CYAN, CLR_RESET);
    printf("%s║%s Zorluk Seviyesi Seciniz:                                                   %s║%s\n", CLR_CYAN, CLR_RESET, CLR_CYAN, CLR_RESET);
    printf("%s║%s   [K] Kolay  : Yuksek kaynak, dusuk hasar, 3 turluk kum firtinasi          %s║%s\n", CLR_CYAN, CLR_RESET, CLR_CYAN, CLR_RESET);
    printf("%s║%s   [N] Normal : Dengeli kaynak, standart esikler, 5 turluk kum firtinasi    %s║%s\n", CLR_CYAN, CLR_RESET, CLR_CYAN, CLR_RESET);
    printf("%s║%s   [Z] Zor    : Kisitli kaynak, zorlu esikler, 7 turluk kum firtinasi       %s║%s\n", CLR_CYAN, CLR_RESET, CLR_CYAN, CLR_RESET);
    printf("%s╚════════════════════════════════════════════════════════════════════════════╝%s\n", CLR_CYAN, CLR_RESET);

    while (!gecerliZorluk) {
        printf("\nSeciminiz (K / N / Z): ");
        if (scanf(" %c", &secim) != 1) {
            tamponTemizle();
            continue;
        }
        tamponTemizle();

        switch (secim) {
            case 'k':
            case 'K':
                oyun.zorluk = 'K';
                oyun.saglik = 100;
                oyun.enerji = 100;
                oyun.oksijen = 100;
                oyun.su = 12;
                oyun.yedekParca = 6;
                oyun.modulHasari = 10;
                oyun.kesifYuksekEsik = 70;
                oyun.kesifDusukEsik = 45;
                oyun.firtinaTurSayisi = 3;
                gecerliZorluk = 1;
                break;

            case 'n':
            case 'N':
                oyun.zorluk = 'N';
                oyun.saglik = 90;
                oyun.enerji = 90;
                oyun.oksijen = 85;
                oyun.su = 8;
                oyun.yedekParca = 4;
                oyun.modulHasari = 20;
                oyun.kesifYuksekEsik = 55;
                oyun.kesifDusukEsik = 35;
                oyun.firtinaTurSayisi = 5;
                gecerliZorluk = 1;
                break;

            case 'z':
            case 'Z':
                oyun.zorluk = 'Z';
                oyun.saglik = 80;
                oyun.enerji = 80;
                oyun.oksijen = 75;
                oyun.su = 5;
                oyun.yedekParca = 2;
                oyun.modulHasari = 30;
                oyun.kesifYuksekEsik = 40;
                oyun.kesifDusukEsik = 25;
                oyun.firtinaTurSayisi = 7;
                gecerliZorluk = 1;
                break;

            default:
                printf("%s[HATA] Gecersiz zorluk seviyesi! Lutfen K, N veya Z giriniz.%s\n", CLR_RED, CLR_RESET);
                break;
        }
    }

    logEkle(&oyun, "Sistem baslatildi. Kurtarma ekibine kadar 15 gun hayatta kalin!");

    /* 2. Ana Komut Dinleme Dongusu (DO-WHILE) */
    char komut = '\0';
    int cikisIstendi = 0;

    do {
        panelCiz(&oyun);
        printf("\nKomut giriniz (G, O, T, D, U, K, H, Q): ");
        if (scanf(" %c", &komut) != 1) {
            tamponTemizle();
            continue;
        }
        tamponTemizle();

        int turGecti = 0;

        /* Buyuk/kucuk harf duyarsiz fall-through SWITCH-CASE */
        switch (komut) {
            case 'g':
            case 'G':
                turGecti = komutKesif(&oyun);
                break;

            case 'o':
            case 'O':
                turGecti = komutOksijenUret(&oyun);
                break;

            case 't':
            case 'T':
                turGecti = komutTamir(&oyun);
                break;

            case 'd':
            case 'D':
                turGecti = komutDurumRaporu(&oyun);
                break;

            case 'u':
            case 'U':
                turGecti = komutDinlen(&oyun);
                break;

            case 'k':
            case 'K':
                turGecti = komutKumFirtinasi(&oyun);
                break;

            case 'h':
            case 'H':
                turGecti = komutHavaKilidi(&oyun);
                break;

            case 'q':
            case 'Q':
                cikisIstendi = 1;
                break;

            default:
                logEkle(&oyun, "[UYARI] Tanimsiz komut! Gecerli komutlar: G, O, T, D, U, K, H, Q");
                turGecti = 0; /* Tanimsiz komut tur harcamaz */
                break;
        }

        if (cikisIstendi) {
            break;
        }

        /* 3. Zaman Akisi ve Oyun Sonu Kontrolu */
        if (turGecti == 1) {
            oyun.tur++;

            /* Once komut sirasinda olum gerceklesti mi kontrol edilir (15. gun son turu dahil) */
            if (oyun.saglik <= 0 || oyun.oksijen <= 0) {
                const char *sebep = (oyun.saglik <= 0)
                                    ? "Muhendisin sagligi tukendi."
                                    : "Istasyondaki oksijen tamamen tukendi.";
                bitisEkraniCiz(&oyun, "GOREV BASARISIZ - KOLONI KAYBEDILDI", sebep);
                return 0;
            }

            /* Her 3 gecerli turda gun sonu / yeni gun gecisi yapilir */
            if (oyun.tur % 3 == 0) {
                /* 15. gunun 3. turu (45. tur) hayatta tamamlandiysa kurtarma ekibi ulasir */
                if (oyun.gun == 15) {
                    bitisEkraniCiz(&oyun, "GOREV BASARILI - KURTARMA EKIBI ULASTI!",
                                   "15 gun boyunca Mars kolonisini hayatta tutmayi basardiniz.");
                    return 0;
                }

                oyun.gun++;

                /* Yeni gun baslangic tuketimi */
                oyun.oksijen -= 8;
                char gunMesaji[128];
                if (oyun.modulHasari > 70) {
                    oyun.saglik -= 5;
                    snprintf(gunMesaji, sizeof(gunMesaji),
                             "=== GUN %d BASLADI === Oksijen -8, Agir Hasar (>70) nedeniyle Saglik -5!",
                             oyun.gun);
                } else {
                    snprintf(gunMesaji, sizeof(gunMesaji),
                             "=== GUN %d BASLADI === Gunluk yasam destek tuketimi: Oksijen -8",
                             oyun.gun);
                }
                sinirlandir(&oyun);
                logEkle(&oyun, gunMesaji);

                /* Yeni gun baslangic tuketimi sonrasi olum kontrolu */
                if (oyun.saglik <= 0 || oyun.oksijen <= 0) {
                    const char *sebep = (oyun.saglik <= 0)
                                        ? "Muhendisin sagligi tukendi."
                                        : "Istasyondaki oksijen tamamen tukendi.";
                    bitisEkraniCiz(&oyun, "GOREV BASARISIZ - KOLONI KAYBEDILDI", sebep);
                    return 0;
                }
            }
        }

    } while (!cikisIstendi && oyun.saglik > 0 && oyun.oksijen > 0 && oyun.gun <= 15);

    /* Q ile cikis durumu */
    bitisEkraniCiz(&oyun, "SIMULASYON SONLANDIRILDI (CIKIS)",
                   "Kullanici Q komutu ile simulasyondan ayrildi.");
    return 0;
}
