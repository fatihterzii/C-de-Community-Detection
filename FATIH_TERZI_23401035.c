#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#define MAX_VERTICES 34
#define RUN_COUNT 50

// =============================================================
// VERİ YAPILARI
// =============================================================

typedef struct
{
    int numVertices;
    int adj[MAX_VERTICES][MAX_VERTICES];
} Graph; // Basit graph yapisi (Fast Greedy ve Ozgun algoritmalar icin)

typedef struct
{
    int numVertices;
    double weights[MAX_VERTICES][MAX_VERTICES];
    double totalWeight;
} LouvainGraph; // Louvain algoritmasi icin agirlikli graph yapisi. Faz2 icin gerekli. Fonksionlari da ayri.

// =============================================================
// YARDIMCI FONKSİYONLAR
// =============================================================

// *** YENİ EKLENEN FONKSİYON: DETAYLI RAPORLAMA ***
void sonuclari_yazdir(const char *algo_adi, int partition[], int n)
{
    printf("\n--- %s SONUCLARI ---\n", algo_adi);

    // Hangi grupların var olduğunu bul
    int groups[MAX_VERTICES];
    int group_count = 0;

    // Mevcut grup ID'lerini tespit et
    for (int i = 0; i < n; i++)
    {
        int g = partition[i]; // Dugumun topluluk ID'si
        bool exists = false;
        for (int j = 0; j < group_count; j++)
            if (groups[j] == g) // Zaten var mi kontrol et
                exists = true;
        if (!exists) // Yoksa yeni grubu ekle
            groups[group_count++] = g;
    }

    printf("Toplam Topluluk Sayisi: %d\n", group_count); 

    // Her grup için üyeleri yazdır
    for (int k = 0; k < group_count; k++) // Her topluluk icin don
    {
        int current_group_id = groups[k]; // Mevcut topluluk ID'si
        printf("  > Topluluk %d: [ ", k + 1); // Ekranda 1'den baslatarak yazdir
        for (int i = 0; i < n; i++)
        {
            if (partition[i] == current_group_id) //Eldeki grup aradigim grupsa
            {
                
                printf("%d ", i + 1); //bastir.
            }
        }
        printf("]\n");
    }
}

Graph *createGraph() // Basit graph olusturucu
{
    Graph *g = (Graph *)malloc(sizeof(Graph)); //Graph icin bellek ayir
    g->numVertices = MAX_VERTICES; //Dugum sayisi
    for (int i = 0; i < MAX_VERTICES; i++)
        for (int j = 0; j < MAX_VERTICES; j++)
            g->adj[i][j] = 0; //Baslangicta tum kenarlar yok
    return g;
}

LouvainGraph *createLouvainGraph(int n) // Louvain graph olusturucu
{
    LouvainGraph *g = (LouvainGraph *)malloc(sizeof(LouvainGraph));
    g->numVertices = n;
    g->totalWeight = 0.0; //Basit graphtan farkli olarak agirlik toplami var. Faz2 icin gerekli.
    for (int i = 0; i < MAX_VERTICES; i++)
        for (int j = 0; j < MAX_VERTICES; j++)
            g->weights[i][j] = 0.0;
    return g;
}

void readGraphData(const char *filename, void *g, int mode) // mode 0: Graph, 1: LouvainGraph. Ortak fonksiyon. Mode ile ayrim yapiliyor.
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("HATA: Dosya yok!\n");
        exit(1);
    }
    int u, v;
    while (fscanf(file, "%d %d", &u, &v) == 2)
    {
        
        if (mode == 0)
        {
            ((Graph *)g)->adj[u][v] = 1; // Undirected graph oldugu icin cift yonlu ekle
            ((Graph *)g)->adj[v][u] = 1;
        }
        else
        {
            if (((LouvainGraph *)g)->weights[u][v] == 0) //Louvain graph icin agirlik ekle
            {
                ((LouvainGraph *)g)->weights[u][v] = 1.0;
                ((LouvainGraph *)g)->weights[v][u] = 1.0;
                ((LouvainGraph *)g)->totalWeight += 2.0; //Cift yonlu oldugu icin 2 ekle
            }
        }
    }
    fclose(file);
}

int getDegree(Graph *g, int node) // Basit graph icin derece hesaplama
{
    int d = 0;
    for (int i = 0; i < g->numVertices; i++)
        if (g->adj[node][i])
            d++;
    return d;
}

double getWeightedDegree(LouvainGraph *g, int node) // Louvain graph icin agirlikli derece hesaplama
{
    double d = 0.0;
    for (int i = 0; i < g->numVertices; i++)
        d += g->weights[node][i];
    return d;
}

void shuffleArray(int *array, int n) //Bir diziyi kendi icinde rastgele karistirir.
{
    for (int i = n - 1; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

double calculateModularity(Graph *g, int partition[]) //Basit graph icin modularity hesaplama
{
    double q = 0.0;
    int m = 0;
    for (int i = 0; i < g->numVertices; i++)
        m += getDegree(g, i); //Edge sayisi icin toplam derece
    m /= 2;
    double two_m = (double)(2 * m); //Modularity formulu icin 2*m
    if (two_m == 0)
        return 0.0;

    for (int i = 0; i < g->numVertices; i++)
    {
        for (int j = 0; j < g->numVertices; j++)
        {
            if (partition[i] == partition[j]) //Ayni toplulukta iseler
            {
                double expected = ((double)getDegree(g, i) * (double)getDegree(g, j)) / two_m; // Beklenen kenar sayisi
                q += (g->adj[i][j] - expected); // Gercek kenar - beklenen kenar, beklenen kenari cikarmadiginda modularity anlamsiz olur.
            }
        }
    }
    return q / two_m; //modularity formulu
}

double calculateLouvainModularity(LouvainGraph *g, int partition[]) //Louvain graph icin modularity hesaplama. agirlik kullaniyor.
{
    double q = 0.0;
    double two_m = g->totalWeight; //Louvain graph icin toplam agirlik
    if (two_m == 0)
        return 0.0;

    for (int i = 0; i < g->numVertices; i++)
    {
        for (int j = 0; j < g->numVertices; j++)
        {
            if (partition[i] == partition[j]) //Ayni toplulukta iseler
            {
                double expected = (getWeightedDegree(g, i) * getWeightedDegree(g, j)) / two_m; //getweighteddegree agirlikli dereceyi dondurur.
                q += (g->weights[i][j] - expected); // Gercek agirlik - beklenen agirlik
            }
        }
    }
    return q / two_m; //modularity formulu
}

// =============================================================
// 1. FAST GREEDY
// =============================================================
double runFastGreedy(Graph *g, int membership[])
{
    int n = g->numVertices;
    for (int i = 0; i < n; i++)
        membership[i] = i; // Baslangicta her dugum kendi toplulugunda kabul edilir.

    double currentQ = calculateModularity(g, membership); // Baslangic modularity degeri. Muhtemelen 0'dan kucuk.

    while (1) //Break gelene kadar devam et 
    {
        double bestQ = -999.0;
        int bestS = -1, bestT = -1;

        for (int i = 0; i < n; i++) // Bir node al.
        {
            for (int j = i + 1; j < n; j++) //O node'un diger node'larla topluluklarini birlestirmeyi dene
            {
                if (membership[i] == membership[j]) //Zaten ayni toplulukta iseler atla
                    continue;

                int tempMem[MAX_VERTICES]; //Yedeklemek icin gecici dizi
                memcpy(tempMem, membership, sizeof(int) * MAX_VERTICES);  // Mevcut topluluk bilgilerini kopyala
                int oldG = membership[i], newG = membership[j]; //Eski ve yeni topluluk ID'leri
                for (int k = 0; k < n; k++)
                    if (tempMem[k] == oldG) //Eski topluluktaki tum node'lari tut.
                        tempMem[k] = newG; //Yeni topluluga tasi
                
                double testQ = calculateModularity(g, tempMem); //Bu halde modularity degerini hesapla
                if (testQ > bestQ) //Eger en iyi modularity ise kaydet
                {
                    bestQ = testQ;
                    bestS = oldG;
                    bestT = newG;
                }
            }
        }

        if (bestQ > currentQ) //Eger bir iyilestirme bulunduysa, topluluklari birlestir
        {
            currentQ = bestQ;
            for (int k = 0; k < n; k++)
                if (membership[k] == bestS) //bestS toplulugundaki tum node'lari al.
                    membership[k] = bestT; //bestT topluluguna tasi
        }
        else
            break; //Artik iyilestirme yoksa cik
    }
    return currentQ; //En iyi modularity degerini dondur
}

// =============================================================
// 2. LOUVAIN ALGORITMASI
// =============================================================

LouvainGraph *rebuildGraph(LouvainGraph *g, int partition[], int num_communities) //Faz 2: Yeni graph olusturma fonsksiyonu
{
    LouvainGraph *newG = createLouvainGraph(num_communities); //Eldeki komün sayisina göre yeni graph olustur
    for (int i = 0; i < g->numVertices; i++)
    {
        for (int j = 0; j < g->numVertices; j++)
        {
            if (g->weights[i][j] > 0) //Eger aralarinda baglanti varsa
            {
                int commI = partition[i]; //i'nin toplulugunu al
                int commJ = partition[j]; //j'nin toplulugunu al
                newG->weights[commI][commJ] += g->weights[i][j]; //Yeni graph'ta bu topluluklar arasina eski toplulukta denk gelen agirligi ekle.
            }
        }
    }
    newG->totalWeight = g->totalWeight; //Toplam agirlik ayni kalir
    return newG; //Yeni graph'i dondur
}

double runLouvain(LouvainGraph *g, int final_partition_output[])
{
    LouvainGraph *currentG = g; //Elimizdeki graph ile basla
    int n = currentG->numVertices; //Elimizdeki graph'in dugum sayisi

    // Global membership takibi (Original Node -> Current Community ID)
    for (int i = 0; i < MAX_VERTICES; i++)
        final_partition_output[i] = i; //Baslangicta her dugum kendi toplulugunda kabul edilir.

    double bestQ = -1.0;
    bool improvement = true;

    while (improvement)
    {
        improvement = false;
        n = currentG->numVertices;
        int local_membership[MAX_VERTICES];
        for (int i = 0; i < n; i++)
            local_membership[i] = i; // Her dugum kendi toplulugunda baslar

        // --- FAZ 1 --- //Louvain 2 fazdan olusur. Bu ilki.
        bool local_change = true; //Local degisiklik var mi?
        while (local_change)
        {
            local_change = false;
            for (int node = 0; node < n; node++)
            {
                int bestComm = local_membership[node]; //Bir node al ve en iyi toplulugu su anki olarak ayarla
                double maxQ = calculateLouvainModularity(currentG, local_membership); //Su anki modularity degerini al
                int originalComm = local_membership[node]; //Orijinal toplulugu sakla

                // Komşuları dene
                int neighborComms[MAX_VERTICES]; // Komsu topluluklari saklamak icin
                int nc_idx = 0;
                for (int nb = 0; nb < n; nb++) //Tum dugumleri komsu kabul et.
                {
                    if (currentG->weights[node][nb] > 0 && local_membership[nb] != originalComm) //Her bir node icin komsuluk kontrolu ve farkli toplulukta olma sarti
                    {
                        bool exists = false; //Zaten eklenmis mi kontrolu
                        for (int k = 0; k < nc_idx; k++) //Tekrar eklememek icin donguyle kontrol et
                            if (neighborComms[k] == local_membership[nb]) //Eger zaten varsa
                                exists = true; //Var
                        if (!exists) //Yoksa 
                            neighborComms[nc_idx++] = local_membership[nb]; //Yeni komsu toplulugu ekle
                    }
                }

                for (int k = 0; k < nc_idx; k++) 
                {
                    int targetComm = neighborComms[k]; //Komsu toplulugu al
                    local_membership[node] = targetComm; //Dugumu o topluluga tasiyor
                    double testQ = calculateLouvainModularity(currentG, local_membership); //Modularity test edelim.
                    if (testQ > maxQ + 0.000001) //Eger modularity artisi sagladiysa (kucuk bir esik degeri ile)
                    {
                        maxQ = testQ; //Yeni en iyi modularity
                        bestComm = targetComm; //En iyi toplulugu guncelle
                    }
                }

                if (bestComm != originalComm) //Eger en iyi topluluk degisti ise, degistir
                {
                    local_membership[node] = bestComm;
                    local_change = true;
                }
                else
                    local_membership[node] = originalComm; //Degismediyse orijinal topluluga geri don
            }
        }

        double currentQ = calculateLouvainModularity(currentG, local_membership); //Faz 1 sonunda elde edilen modularity degeri
        if (currentQ > bestQ) //Eger global en iyi modularity ise guncelle
            bestQ = currentQ;

        // Global membership'i guncelle
        for (int i = 0; i < MAX_VERTICES; i++)
        {
            // Şimdiki grubu (super-node ID'si) al
            int current_super_node = final_partition_output[i];
            // O super-node bu turda nereye gitti?
            if (current_super_node < n)
            {
                final_partition_output[i] = local_membership[current_super_node];
            }
        }

        // --- FAZ 2 ---
        int renumber[MAX_VERTICES]; // Topluluk ID'lerini sikistirmak icin kullaniyoruz
        for (int i = 0; i < MAX_VERTICES; i++)
            renumber[i] = -1; // Baslangicta tum topluluklar -1 (atanmamis)
        int next_comm_id = 0; // Yeni topluluk ID'si

        for (int i = 0; i < n; i++) // Mevcut topluluklari sikistir
        {
            if (renumber[local_membership[i]] == -1) //Eger bu topluluk daha once atanmadiyse
                renumber[local_membership[i]] = next_comm_id++; //Yeni bir ID ata ve arttir. next_comm_id bir sonraki komün sayisini tutar.
        }

        for (int i = 0; i < n; i++)
            local_membership[i] = renumber[local_membership[i]]; // Local membership'i sikistirilmis ID'lerle guncelle

        // Final partition'daki ID'leri de sıkıştırılmış yeni ID'lere güncelle
        for (int i = 0; i < MAX_VERTICES; i++)
        {
            final_partition_output[i] = renumber[final_partition_output[i]]; // Final partition'daki ID'leri de guncelle
        }

        if (next_comm_id == n) //Eger topluluk sayisi degismediyse artik ilerleme yok demektir.
        {
            improvement = false;
        }
        else
        {
            LouvainGraph *nextG = rebuildGraph(currentG, local_membership, next_comm_id); //Sikistirilmis topluluklarla yeni graph olustur
            if (currentG != g) //Eger mevcut graph orijinal graph degilse, bellegi serbest birak
                free(currentG);
            currentG = nextG; //Yeni graph'i mevcut graph yap ve faz 1'e geri don
            improvement = true;
        }
    }

    if (currentG != g)
        free(currentG);
    return bestQ;
}

// =============================================================
// 3. ÖZGÜN ALGO 1 (SABİT TAHMİN)
// =============================================================
double runOriginalFixedK(Graph *g, int output_partition[])
{
    int edge_count = 0;
    for (int i = 0; i < g->numVertices; i++)
        edge_count += getDegree(g, i);
    edge_count /= 2;
    double avg_degree = (double)(2 * edge_count) / g->numVertices; 

    int estimated_k = (int)(sqrt(g->numVertices) / (avg_degree / 2.0)); //Sezgisel yaklasimla sabit bir k tahmini yapiyorum.
    if (estimated_k < 2) // Minimum 2 topluluk. Tek topluluk anlamsiz.
        estimated_k = 2;

    double bestGlobalQ = -1.0;

    for (int run = 0; run < RUN_COUNT; run++)
    {
        int partition[MAX_VERTICES];
        for (int i = 0; i < g->numVertices; i++)
            partition[i] = rand() % estimated_k; // Rastgele baslangic. 0 ile k-1 arasinda. k=2 icin 0 veya 1.

        double currentQ = calculateModularity(g, partition);
        bool improved = true;
        int iter = 0;
        int order[MAX_VERTICES];
        for (int i = 0; i < g->numVertices; i++)
            order[i] = i;

        while (improved && iter < 10) // 10 iterasyonla sinirlayorum. 1000 de denendi. Pek fark yok.
        {
            improved = false;
            shuffleArray(order, g->numVertices); //dugumleri rastgele siraladim.
            for (int k = 0; k < g->numVertices; k++) 
            {
                int node = order[k]; //rastgele dugum aldim.
                int bestComm = partition[node]; //su anki topluluk. local maximizasyonda bakalim degisecek mi?
                double localMaxQ = currentQ; //su anki modularity
                int originalComm = partition[node]; //orijinal topluluk

                for (int nb = 0; nb < g->numVertices; nb++) //Local maximize icin rastgele gelen node'un tum komsularina bakiyorum.
                {
                    if (g->adj[node][nb]) //Eger aralarinda baglanti varsa!! (modularity artisi icin onemli)
                    {
                        int targetComm = partition[nb];
                        if (targetComm != originalComm) //Ayni topluluga gecmeye gerek yok. Farkli topluluklari dene.
                        {
                            partition[node] = targetComm; //Dugumu hedef topluluga tasiyorum.
                            double newQ = calculateModularity(g, partition); //Modularity hesapla.
                            if (newQ > localMaxQ) //Localde maximize et.
                            {
                                localMaxQ = newQ;
                                bestComm = targetComm;
                                improved = true;
                            }
                            else
                                partition[node] = originalComm;
                        }
                    }
                }
                if (partition[node] != bestComm) //Eger en iyi topluluk degisti ise, degistir ve currentQ'yu guncelle.
                {
                    partition[node] = bestComm;
                    currentQ = localMaxQ;
                }
            }
            iter++;
        }
        if (currentQ > bestGlobalQ) //Bu run'daki en iyi sonucu global en iyi ile karsilastir.
        {
            bestGlobalQ = currentQ;
            for (int i = 0; i < MAX_VERTICES; i++)
                output_partition[i] = partition[i];
        }
    }
    return bestGlobalQ; //En iyi global modularity degerini dondur.
}

// =============================================================
// 4. ÖZGÜN ALGO 2 (DİNAMİK ARALIK)
// =============================================================
double runOriginalRangeK(Graph *g, int output_partition[])
{
    int k_min = 2; //Tek topluluk anlamsiz.
    int k_max = (int)(sqrt(g->numVertices) * 1.5); //Daha genis aralikta denemek icin 1.5 ile carptim.
    if (k_max < 3) //En azindan 2 ve 3 arasinda olsun.
        k_max = 3;

    double bestGlobalQ = -1.0;

    for (int run = 0; run < RUN_COUNT; run++) //Daha stabil sonuclar icin birden fazla deneme.
    {
        int current_k = k_min + rand() % (k_max - k_min + 1); //rastgele k secimi
        int partition[MAX_VERTICES];
        for (int i = 0; i < g->numVertices; i++)
            partition[i] = rand() % current_k; //gruplama baslangici. bu da rastgele. k=4 icin 0,1,2,3 olabilir.

        double currentQ = calculateModularity(g, partition);
        bool improved = true;
        int iter = 0;
        int order[MAX_VERTICES]; //node sirasi icin dizi
        for (int i = 0; i < g->numVertices; i++)
            order[i] = i;

        while (improved && iter < 50)
        {
            improved = false;
            shuffleArray(order, g->numVertices); //node sirasi rastgele olsun
            for (int k = 0; k < g->numVertices; k++)
            {
                int node = order[k]; //rastgele node aldim
                int bestComm = partition[node]; //su anki topluluk
                double localMaxQ = currentQ; //su anki modularity
                int originalComm = partition[node]; //orijinal topluluk

                for (int nb = 0; nb < g->numVertices; nb++) //Local maximize icin rastgele gelen node'un tum komsularina bakiyorum.
                {
                    if (g->adj[node][nb]) //Eger aralarinda komsuluk varsa!! (modularity artisi icin onemli)
                    {
                        int targetComm = partition[nb];
                        if (targetComm != originalComm) //Ayni topluluga gecmeye gerek yok. Farkli topluluklari dene.
                        {
                            partition[node] = targetComm;
                            double newQ = calculateModularity(g, partition); //Modularity hesapla.
                            if (newQ > localMaxQ) //Localde maximize et.
                            {
                                localMaxQ = newQ;
                                bestComm = targetComm;
                                improved = true;
                            }
                            else
                                partition[node] = originalComm; //Modularity artmadi. Geri al
                        }
                    }
                }
                if (partition[node] != bestComm) //Eger en iyi topluluk degisti ise, degistir ve currentQ'yu guncelle.
                {
                    partition[node] = bestComm;
                    currentQ = localMaxQ;
                }
            }
            iter++;
        }
        if (currentQ > bestGlobalQ) //Bu run'daki en iyi sonucu global en iyi ile karsilastir.
        {
            bestGlobalQ = currentQ;
            for (int i = 0; i < MAX_VERTICES; i++)
                output_partition[i] = partition[i];
        }
    }
    return bestGlobalQ;
}
/*
2. Ozgun algoritma ilk algoritmadan farkli olarak, k degerini sabit tutmak yerine dinamik olarak belirler.
Bu sayede, topluluk sayisi icin daha esnek bir yaklasim sunar ve genellikle daha iyi modularity degerleri elde eder. 
1. algoritmanin optimize hali gibi dusunulebilir. ancak 10 milyon nodelu veri setlerinde 2. algoritmayi kullanmak pratik olmayabilir.
Cunku k'nin araligi genisledikce, denenecek k degerleri artar ve bu da hesaplama surelerini uzatir. Zachary 34 node oldugu icin burada sorun olmaz.
*/


// =============================================================
// MAIN
// =============================================================
int main()
{
    srand(time(NULL));
    const char *FILENAME = "zachary_data.txt";

    printf("\n==============================================\n");
    printf("     4 ALGORITMA DETAYLI KARSILASTIRMASI      \n");
    printf("==============================================\n");

    // GRAFLARI OLUSTUR
    Graph *gSimple = createGraph();
    readGraphData(FILENAME, gSimple, 0);

    LouvainGraph *gLouvain = createLouvainGraph(MAX_VERTICES);
    readGraphData(FILENAME, gLouvain, 1);

    // 1. FAST GREEDY
    int parts1[MAX_VERTICES];
    double q1 = runFastGreedy(gSimple, parts1);
    sonuclari_yazdir("FAST GREEDY", parts1, MAX_VERTICES);

    // 2. LOUVAIN
    int parts2[MAX_VERTICES];
    double q2 = runLouvain(gLouvain, parts2);
    sonuclari_yazdir("LOUVAIN", parts2, MAX_VERTICES);

    // 3. OZGUN 1 (SABIT)
    int parts3[MAX_VERTICES];
    double q3 = runOriginalFixedK(gSimple, parts3);
    sonuclari_yazdir("OZGUN ALGO 1 (SABIT TAHMIN)", parts3, MAX_VERTICES);

    // 4. OZGUN 2 (ARALIK)
    int parts4[MAX_VERTICES];
    double q4 = runOriginalRangeK(gSimple, parts4);
    sonuclari_yazdir("OZGUN ALGO 2 (DINAMIK ARALIK)", parts4, MAX_VERTICES);

    // SONUC TABLOSU
    printf("\n\n");
    printf("==============================================\n");
    printf("| %-25s | %-10s |\n", "ALGORITMA", "MODULARITY");
    printf("|---------------------------|------------|\n");
    printf("| Fast Greedy               | %.4f     |\n", q1);
    printf("| Louvain                   | %.4f     |\n", q2);
    printf("| Ozgun (Sabit Tahmin)      | %.4f     |\n", q3);
    printf("| Ozgun (Dinamik Aralik)    | %.4f     |\n", q4);
    printf("==============================================\n");

    free(gSimple);
    free(gLouvain);
    return 0;
}