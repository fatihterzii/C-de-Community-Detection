Zachary Karate Club Topluluk Tespiti ve Algoritma Karşılaştırması
Bu proje, ünlü Zachary Karate Club sosyal ağ verisi üzerinde topluluk tespiti (community detection) yapan dört farklı algoritmanın C dilindeki uygulamasıdır. Proje, literatürde
bilinen algoritmalar ile bu proje için geliştirilmiş özgün sezgisel (heuristic) algoritmaların performansını Modularity (Q) skoru üzerinden karşılaştırır.
Proje FATIH_TERZI_23401035.c dosyası içerisinde aşağıdaki 4 algoritmayı birleştirir.
1) FastGreedy (Clauset-Newman-Moore)
2) Louvain Algoritması
3) Sabit k topluluk tahminli algoritma (1. Özgün Algoritma)
4) Dinamik k topluluk tahminli algoritma(2. Özgün Algoritma)(İlkinin daha optimize versiyonu)

Kurulum Ve Derleme
Bu proje standart C kütüphaneleri kullanır. Derlemek için GCC derleyicisine ihtiyacınız vardır. Matematik kütüphanesini (math.h) bağlamak için -lm bayrağını kullanmayı unutmayın.

Derleme Komutu
Terminal veya komut satırında şu komutu çalıştırın: gcc FATIH_TERZI_23401035.c -o community_detection -lm

Çalıştırma
Derleme işlemi başarılı olduktan sonra programı çalıştırın:
Windows : community_detection.exe
Linux/Mac : ./community_detection

Veri Seti Formatı (zachary_data.txt)
Programın çalışabilmesi için çalıştırılabilir dosyanın (exe) olduğu dizinde zachary_data.txt adında bir dosya bulunmalıdır.

Gereksinimler
1) GCC Compiler
2) C Standart Kütüphaneleri (stdlib, stdio, math, time, string, stdbool)

Lisans
Bu proje eğitim ve akademik araştırma amaçlı geliştirilmiştir. Açık kaynaklıdır.
