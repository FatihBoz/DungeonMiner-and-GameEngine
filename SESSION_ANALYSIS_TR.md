# DungeonMiner Session Analizi

## Kapsam

Bu not, `D:\hexas_files\gamedev\cgame\DungeonMiner-and-GameEngine` içinde bu oturumda yapılan işleri ve özellikle şu üç commit'in ne değiştirdiğini açıklar:

- `fac663386b318f1716789468683c044aee8a4a52` - `bat and ghost and debug`
- `f37c50f9bd3faa1b1283092803e829254a8ec7ef` - `new tankgolem and skeleton enemy`
- `fa263fc0fd1f75d2f186a22755506490f149da88` - `tankgolem, bat, skeleton and ghost completed.`

## Kısa Özet

Bu oturumda eski Win32 C++ oyun iskeletine bir düşman mimarisi eklendi ve bu mimari birkaç dalga halinde olgunlaştırıldı:

- `Enemy` taban sınıfı ile ortak AI durum makinesi kuruldu.
- `BatEnemy`, `GhostEnemy`, `TankGolemEnemy`, `SkeletonEnemy` ve `SkeletonProjectile` gibi alt sınıflar ile farklı davranışlar ayrıştırıldı.
- `DebugSystem` ile çalışma anında log, phase, exception ve heartbeat takibi eklendi.
- `AStarPathfinder` ile grid tabanlı yol bulma eklendi.
- `Sprite` ve `Bitmap` katmanı, çok satırlı sprite sheet'ler, flip davranışı, ölçekleme ve daha sonra rotasyon gibi ihtiyaçları destekleyecek şekilde genişletildi.
- Golem, bat, skeleton ve projectile davranışları kullanıcı geri bildirimiyle tekrar tekrar düzeltildi.

## Sistem Mimarisi

### 1. Temel Düşman Mimarisi

`Enemy`, doğrudan `Sprite` üstüne kurulan ortak bir AI tabanı oldu. Mantık şu eksene ayrıldı:

- Durum yönetimi: `IDLE`, `PATROL`, `CHASE`, `FORGET`, `ATTACK`
- Ortak parametreler: sağlık, hareket hızı, algılama mesafesi, unutma mesafesi, saldırı mesafesi
- Ortak yardımcılar:
  - oyuncuya uzaklık hesaplama
  - merkez nokta hesaplama
  - hedefe doğru hareket etme
  - durma
  - oyuncu algılama ve saldırı menzili kontrolü

Bu tasarımın önemi şuydu: her düşman için davranış tamamen yeniden yazılmadı. Ortak yaşam döngüsü `Enemy::Update()` içinde tutuldu, alt sınıflar sadece kendi durumlarını özelleştirdi.

### 2. Sprite ve Bitmap Katmanı

`Sprite` tarafı ilk başta klasik tek frame kullanımına yakındı, ama oturum boyunca sprite sheet temelli animasyon ve görsel dönüşümler için genişletildi:

- çok satırlı frame desteği
- frame satırı seçimi
- frame sayısı ve row bazlı çizim
- yatay flip
- ölçekleme
- daha sonra projectile için bitmap rotasyonu

`Bitmap` tarafında da parça çizme, ölçekleme, flip ve son aşamada merkez etrafında rotasyon eklendi.

Bu sayede düşman sınıfları kendi çizim mantığını taşıyabildi, fakat render altyapısı ortak kaldı.

### 3. Debug Sistemi

`DebugSystem`, ilk commit'te sadece log almak için değil, crash analizi için de kuruldu. Sistem şu işleri yaptı:

- aktif/pasif çalışabilen log hattı
- phase takibi
- frame heartbeat
- CRT error hook
- unhandled exception filter
- vectored exception handler

Buradaki ana fikir şuydu: oyundaki crash'i "son görülen davranış" yerine "hangi fazda ve hangi çerçevede patladığı" üzerinden izlemek.

## Düşmanların Nasıl Yapıldığı

### BatEnemy

Bat, ilk başta random hareket eden bir düşman olarak tasarlandı. Sonrasında birkaç kez düzeltildi:

- tek bir hedef seçip ona gitme
- hedeften hızlıca yeniden kaçmama
- takılı kalma algılama
- açık zemin önceliği
- harita sınırları içinde kalma
- kullanıcı isteğiyle oyuncuya kolay görülebilmesi için başlangıçta daha yakın spawn

Bat, oyuncuyu kovalayan bir düşman olarak bırakılmadı. Bu önemliydi; çünkü random hareket ve keşif hissi veren, hafif ama tehditkâr bir yan düşman olarak tutuldu.

### GhostEnemy

Ghost, Bat benzeri ama duvarlardan geçebilen bir düşman oldu. Buradaki fark davranış katmanında:

- map collision bazı sürücüler için devre dışı bırakıldı
- random movement korundu
- sprite scale 2x yapıldı
- sola giderken yatay flip eklendi

Yani Ghost, fiziksel kısıtları delen bir "mobil tehdit" olarak konumlandı.

### TankGolemEnemy

Tank Golem oturumun en karmaşık düşmanı oldu. Davranış katmanında üç ayrı mod birleşti:

1. Patrol
2. Chase
3. Fallback move

Temel özellikleri:

- yüksek sağlık
- düşük hareket hızı
- güçlü ve dayanıklı tasarım
- 4 yönlü sprite sheet
- 3 frame'lik yön animasyonu
- 2x scale

Patrol tarafında golem bir yönde yürür, engel veya sıkışma olursa yön değiştirir.

Chase tarafında oyuncu algılanırsa A* ile yol aranır. Yol varsa o yol boyunca yürür. Yol yoksa "donup kalmak" yerine normal patrol hareketine döner. Bu fallback, kullanıcı isteğiyle özellikle korundu.

Bu düşmanda iki kritik davranış daha eklendi:

- path refresh periyodu ile sürekli yeniden hesaplama yerine kontrollü güncelleme
- durduğunda son yön satırının orta karesi üzerinde kalma

### SkeletonEnemy

Skeleton, bat/golem hattından ayrışan başka bir davranış modeli oldu:

- iki nokta arasında patrol
- oyuncuyu görürse durup ateş etme
- oyuncuyu kovalamama
- projectile spawn etme

Burada düşman "yakın dövüş tehditi" değil, "uzaktan baskı" birimi olarak tasarlandı. Bu yüzden state machine içinde attack state merkezdeydi.

## Pathfinding Nasıl Yapıldı

`AStarPathfinder`, grid tabanlı ve callback tabanlı yazıldı.

### Girdi Modeli

Pathfinder'a şu bilgiler verildi:

- başlangıç row/col
- hedef row/col
- grid boyutu
- engelli tile sorgusu yapan callback
- context pointer

Bu tasarımın iyi tarafı, pathfinder'ın map veri yapısına doğrudan bağlanmamasıydı. `AStarPathfinder` sadece "şu tile bloklu mu?" sorusunu soruyor.

### Algoritma

Kullanılan yöntem klasik A* idi:

- Node başına `g`, `h`, parent, open, closed tutuldu.
- Heuristic olarak Manhattan distance kullanıldı.
- Sadece 4 yönlü komşular değerlendirildi.
- En düşük `g + h` değerine sahip açık node seçildi.
- Hedefe ulaşınca parent zincirinden path rebuild edildi.

### Tank Golem ile Entegrasyon

Tank Golem path bulurken önce oyuncu ve düşman hücrelerini tile koordinatına çevirdi. Sonra:

- başlangıç ve hedef için yakın açık tile aradı
- eğer uçlardan biri blokluysa en yakın açık tile'a kaydı
- A* path bulamazsa fallback patrol hareketine geri döndü

Bu, pathfinding'in "başarısız olursa karakteri dondurma" yerine "oynanabilirliği koru" yaklaşımıyla bağlanmasını sağladı.

### Neden Köşe Sorunu Çıktı

Sonradan görülen köşe takılmaları, hedef tile'a merkezden merkez hareket ederken grid hizası ile sprite merkezinin tam örtüşmemesinden kaynaklandı. Bu yüzden golem tarafında:

- path refresh,
- tile alignment,
- center snapping,
- fallback direction logic

bir arada çalıştı.

## Projectile Sistemi

`SkeletonProjectile`, straight-line projectile olarak kuruldu.

### Davranış

- spawn anında hedefe giden velocity hesaplandı
- sonra bu velocity her frame yeniden üretilmedi
- wall hit, player hit ve bounds kill ile yok edildi

Bu değişiklik kritik oldu; çünkü ilk versiyonda projectile hedefe doğru her frame yeniden yönlendirildiğinde, hedefi kaçırınca aynı noktada titreyip asılı kalabiliyordu.

### Görselleştirme

İlk aşamada projectile sadece bir sprite olarak çizildi. Son aşamada:

- `bitmaps/bone.bmp` kullanıldı
- projectile Z ekseni etrafında döndürülür hale getirildi
- rotasyon `Bitmap` seviyesinde ortak bir çizim yardımcı fonksiyonuyla yapıldı

Bu yaklaşımın avantajı, projectile'a özel görsel davranışın `Sprite` tabanını kirletmemesi oldu.

## Üç Commit Ne Değiştirdi

### `fac663386b318f1716789468683c044aee8a4a52`

Bu commit temel düşman sisteminin başlangıç noktasıydı.

Ne yaptı:

- `AGENTS.md` eklendi
- `Enemy` taban sınıfı eklendi
- `BatEnemy` eklendi
- `GhostEnemy` eklendi
- `DebugSystem` eklendi
- `Roids.cpp` ve `GameEngine` içine düşman/sistem entegrasyonu girdi
- proje dosyaları güncellendi

Sistemsel anlamı:

- düşmanlar artık tekil script benzeri kodlar değil, ortak bir AI tabanına sahip sınıflar haline geldi
- debug sistemi, daha sonraki crash ve heap corruption analizlerinin altyapısı oldu
- oyun döngüsü, düşman update/collision akışını taşıyacak şekilde genişledi

Bu commit, "enemy framework"ün çekirdeğini attı.

### `f37c50f9bd3faa1b1283092803e829254a8ec7ef`

Bu commit düşman sistemini ciddi biçimde büyüttü.

Ne yaptı:

- `AStarPathfinder` eklendi
- `TankGolemEnemy` eklendi
- `SkeletonEnemy` eklendi
- `SkeletonProjectile` ve projectile math helper eklendi
- `Sprite` ve `Bitmap` animasyon/draw tarafı genişledi
- test dosyaları eklendi
- yeni bitmap asset'leri eklendi

Sistemsel anlamı:

- patrol/chase/attack ayrımı artık gerçek düşman çeşitliliğine dönüştü
- Tank Golem ile yol bulma ihtiyacı ortaya çıktı
- Skeleton ile ranged combat ve projectile yaşam döngüsü kuruldu
- sprite/bitmap katmanı çok satırlı animasyon ve çizim farklarını taşıyacak seviyeye yükseldi

Bu commit, düşmanların "davranış olarak farklı" hale geldiği noktaydı.

### `fa263fc0fd1f75d2f186a22755506490f149da88`

Bu commit davranışları tamamlayan ve görsel/sistemsel cilayı yapan aşamaydı.

Ne yaptı:

- Bat, Tank Golem, Skeleton ve Ghost davranışları tamamlandı
- `Bitmap` tarafında ek çizim desteği geldi
- `Player` ve `GameEngine` tarafında küçük entegrasyonlar yapıldı
- projectile, bat ve golem çizim/animasyon tarafı daha tutarlı hale getirildi
- yeni bitmap dosyaları eklendi

Sistemsel anlamı:

- önceki commit'lerde atılan iskeletler gerçek oyun davranışına dönüştü
- görsel animasyon ile hareket yönü eşleştirildi
- projectile ve düşman çizimlerinde asset seviyesinde netleşme sağlandı

Bu commit, sistemin "çalışıyor" aşamasından "oynanabilir ve okunabilir" aşamasına geçişiydi.

## Oturumda Yapılan Ek Düzeltmeler

Bu üç commit'in üstüne oturum sırasında birkaç kritik düzeltme daha yapıldı:

- joystick polling yorum satırına alındı, kod silinmedi
- projectile hedefe yeniden yönlendirilmesi kaldırıldı
- projectile çarpışma sonrası titreme ve sonsuz asılı kalma davranışı düzeltildi
- projectile görseli Z ekseni rotasyonu ile güncellendi
- Bat ve Skeleton için durunca orta karede kalma davranışı eklendi
- Tank Golem için duruşta orta kare, path başarısızlığında fallback patrol davranışı korundu

## Bu Oturumdan Çıkan Asıl Tasarım Kararları

- Ortak düşman davranışı `Enemy` altında toplanmalı
- Düşmanların farkı, state machine ve alt sınıf override'ları ile kurulmalı
- Pathfinding map'e sıkı bağlanmamalı, callback ile tile blok kontrolü almalı
- Projectile homing değilse velocity spawn'da sabitlenmeli
- Sprite animasyonu, yön ve frame yönetimi ile davranışa bağlanmalı
- Debug sistemi kapatılabilir olmalı ve kapalıyken performansa yük olmamalı

## Gelecek İçin Not

Bu projede en kırılgan yerler şunlar:

- sprite sheet frame boyutları
- scale uygulanmış sprite collision ve center hesapları
- pathfinding ile tile center hizası
- projectile rotasyon çizimi
- shutdown / cleanup sırası

Bir sonraki çalışma bunlardan birine girerse, önce ilgili sınıf zinciri okunmalı:

- `Sprite -> Enemy -> Bat/Ghost/TankGolem/Skeleton`
- `Bitmap -> projectile drawing`
- `AStarPathfinder -> TankGolemEnemy`
- `DebugSystem -> GameEngine / Roids`

## Ek Teknik Derinlik

### Update Zinciri

Oyun içinde düşmanların çalışması tek bir "oyuncu gördü mü" kontrolünden ibaret değildi. Zincir şu şekilde ilerledi:

1. `Roids.cpp` düşmanı yaratır ve oyun listesine ekler.
2. `GameEngine::UpdateSprites()` her frame tüm sprite'ları günceller.
3. `Enemy::Update()` ortak state machine'i çalıştırır.
4. Alt sınıf override'ı kendi kararını uygular.
5. `Sprite::Update()` fiziksel pozisyonu ve bounds davranışını işler.
6. Collision ve map kontrolü gerektiğinde ayrı callback'lerden geçer.
7. `Sprite::Draw()` veya override edilmiş çizim fonksiyonu render üretir.

Bu akışın en önemli sonucu, davranış, hareket ve render katmanlarının birbirine karışmaması oldu.

### Render Zinciri

Render tarafında üç seviye var:

- `Sprite` davranış seviyesinden frame seçer.
- `Bitmap` GDI çizimini yapar.
- özel durumlarda `DrawPartScaledFlipped` veya `DrawScaledRotated` gibi yardımcılar kullanılır.

Bu sayede Bat, Golem, Skeleton ve projectile aynı temel render altyapısını kullanırken birbirinden farklı görsel kurallara sahip olabildi.

### Veri Modeli

Bu sistemde önemli veri tipleri:

- `RECT`: sprite position ve collision alanı
- `POINT`: velocity, target, tile coordinate
- `std::vector<POINT>`: patrol route ve path
- `AStarNode`: pathfinding sırasında açık/kapalı node durumu
- `EnemyAIState`: AI state machine enumerasyonu

Yani oyun mantığı aslında üç koordinat uzayı arasında gidip geliyor:

- piksel uzayı
- tile/grid uzayı
- frame/sprite sheet uzayı

Tank Golem ve Skeleton tarafındaki hataların çoğu, bu uzaylardan birinin diğerine yanlış çevrilmesinden kaynaklanıyordu.

### Tank Golem Teknik Özeti

Tank Golem'i sunumda teknik olarak şöyle anlatabilirsin:

- Hedefi doğrudan kovalamıyor, önce path buluyor.
- Path varsa tile merkezleri üzerinden yürüyor.
- Path yoksa patrol yönünü koruyup devam ediyor.
- Engel veya sıkışma algılanırsa yön değiştiriyor.
- Duruşta orta frame'e kilitleniyor.
- Animasyon row'u hareket yönünden türetiliyor.

Bu, zeki görünen ama düşmeyen bir ağır düşman üretmek için gerekli olan minimum mühendislik setiydi.

### Skeleton Teknik Özeti

Skeleton için teknik ayrım şuydu:

- hareket ettiğinde patrol path'i izliyor,
- oyuncu görünce hareketi kesiyor,
- ateş etmeye geçiyor,
- projectile yönünü player center'a göre hesaplıyor,
- projectile isabet etmediğinde de fiziksel olarak sonsuza kadar sürüklenmiyor.

Bu davranış, ranged enemy için "yapışkan" değil "tehdit oluşturup yerinde duran" bir model sağladı.

### Projectile Teknik Özeti

Bone projectile için iki katmanlı çözüm kullanıldı:

- hareket katmanı: spawn anında sabit velocity
- görsel katman: merkez etrafında rotate edilmiş bitmap

Bu çözümün sunumdaki önemli cümlesi şu:

"Projectile homing değildir; bu yüzden her frame hedef güncellenmez. Hareket bir kez hesaplanır, render ise kendi ekseni etrafında döner."

### Debug Sistemi Teknik Özeti

Debug hattı, crash'i sadece loglamak için değil, kök neden analizi için kuruldu. Özellikle:

- phase etiketi ile hangi fonksiyon hattında olunduğu görüldü
- heartbeat ile oyunun gerçekten akıp akmadığı doğrulandı
- exception handler ile access violation ve heap corruption son anı kayda geçti
- disabled/enabled modeli ile performans yükü kontrol edildi

Bu yapı, "log var" seviyesinden "hangi sistem parçası patladı" seviyesine çıktı.

### Commit Bazlı Daha Teknik Okuma

`fac6633`:

- enemy framework'ün tabanını kurdu
- debug system'i getirdi
- oyunun runtime gözlemi için phase/exception altyapısı açtı

`f37c50f`:

- pathfinding ekledi
- Tank Golem'i grid-aware hale getirdi
- Skeleton projectile hattını başlattı
- `Sprite` ve `Bitmap` katmanını animasyon için genişletti

`fa263fc`:

- animasyon ve asset seçimlerini tamamladı
- projectile ve enemy davranışlarını stabilize etti
- render tarafını daha okunur hale getirdi

### Sunum İçin Tek Cümlelik Mimari Özet

"Bu sistemde her düşman, ortak bir `Enemy` state machine'i üzerinde koşuyor; hareket `Sprite`, görsel paketleme `Bitmap`, yol bulma `AStarPathfinder`, hata analizi `DebugSystem`, özel davranışlar ise alt sınıflar tarafından yönetiliyor."

