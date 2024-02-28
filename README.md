# Parallel Mergesort
###### Implementierung von Joël Allemann

Im Rahmen des Moduls PAC (parallel computing) an der FHNW wird als Assignment ein Mergesort implementiert. Dieser soll, entsprechend dem Modul, parallel implementiert werden. Dies soll über zwei Wege geschehen. Eine, welche mit [Konventionellen C++ Threads](#konventionelle-threads) arbeitet, und eine welche mit der [Open Multi Processing](#openmp) API arbeitet.

## Implementierung
Die Implementierung der beiden Versionen basiert auf dem selben Prinzip.
```cpp
uint64_t parallelChunks = a.size() / NO_OF_THREADS;
```
Zuerst wird berechnet, wie die Thread sich die Arbeit teilen. Jeder Thread bearbeitet parallel eine Chunk der ganzen Menge. Die Grösse eines solchen Chunks ist $\frac{|Menge|}{n_{Threads}}$ - so können $n_{Threads}$ an gleich grossen Stücken arbeiten.

```cpp
for (size_t i = 0; i < NO_OF_THREADS; i++) {
    // start sorting for each chunk
    // beginning at i * parallelChunks
    // and ending at (i + 1) * parallelChunks
}
```
In einem nächsten Schritt werden dann die Threads gestartet, welche dann die bekannte Implementierung eines Mergesorts ausführen.

Wenn alle Threads ihre Arbeit beendet haben erhält man eine Menge fast fertig sortierte Menge - die einzelnen Chunks sind in sich schon sortiert.
```
|sorted|sorted|sorted|sorted|sorted|sorted|sorted|sorted|
```
Im letzen Schritt muss diese in sich sortierte Menge noch gemerged werden. Dies wird mit folgendem Code (ebenfalls parallel) erledigt:
```cpp
size_t nFinalizingMerges = NO_OF_THREADS / 2;
while (nFinalizingMerges > 0) {
    uint64_t mergeFrame = a.size() / nFinalizingMerges;
    for (size_t i = 0; i < nFinalizingMerges; i++) {
        uint64_t beg = i * mergeFrame;
        // merge with
        // beg = beg
        // m   = beg + mergeFrame / 2
        // end = beg + mergeFrame
    }
    nFinalizingMerges /= 2;
}
```
Die Anzahl finalisierende Merges `nFinalizingMerges` ist die Anzahl Merges, welche noch ausgeführt werden müssen. Da immer zwei schon sortiere Chunks zusammengefügt werden, und `NO_OF_THREADS` sortierte Chunks existieren, müssen im nächsten Schritt $\frac{n_{Threads}}{2}$ merges ausgeführt werden.

```
|-- sorted ---|-- sorted ---|-- sorted ---|-- sorted ---|
|---------- sorted ---------|---------- sorted ---------|
|----------------------- sorted ------------------------|
```

Wenn alle Chunks der Menge gemerged wurden, gibt es doppelt so grosse in sich sortierte Chunks - bzw. noch halb so viele Chunks welche gemerged werden müssen.

Diese Schritte werden so lange wiederholt bis keine finalisierenden Merges (also `nFinalizingMerges` == 0 ist) mehr gemacht werden müssen. Danach ist die Sortierung fertig.

### Konventionelle Threads
Um mit konventionellen C++ Threads zu arbeiten, muss jeder Thread einzeln mit seinem Arbeitspaket gestart werden.
In der parallelen Sortierungsphase, kann dies folgendermassen erreicht werden:

```cpp
std::thread threadpool[NO_OF_THREADS];
for (size_t i = 0; i < NO_OF_THREADS; i++) {
    threadpool[i] = std::thread(mSort, std::ref(a), i * parallelChunks, (i + 1) * parallelChunks);
}

for (size_t i = 0; i < NO_OF_THREADS; i++) {
    threadpool[i].join();
}
```
Wichtig ist, die Threads im threadpool auch wieder zu joinen.

Bei den finalisierenden Merges müssen die Threads in einem ähnlichen Stil gestartet werden:
```cpp
for (size_t i = 0; i < nFinalizingMerges; i++) {
    uint64_t beg = i * mergeFrame;
    threadpool[i] = std::thread(mMerge, std::ref(a), beg, beg + mergeFrame / 2, beg + mergeFrame);
}
for (size_t i = 0; i < nFinalizingMerges; i++) {
    threadpool[i].join();
}
```

### OpenMP
Mit OpenMP können die Threads noch einfacher wie die konventionellen Threads generiert werden. Die Sortierung sieht dann folgendermassen aus:
```cpp
#pragma omp parallel for num_threads(NO_OF_THREADS)
for (size_t i = 0; i < NO_OF_THREADS; i++) {
    mSort(a, i * parallelChunks, (i + 1) * parallelChunks);
}
```
Dies startet `NO_OF_THREADS` threads, welche dann die `mSort` Funktion mit den richtigen Parametern aufruft.

Auch hier sieht das finale Mergen ähnlich aus:
```cpp
#pragma omp parallel for num_threads(NO_OF_THREADS)
for (size_t i = 0; i < nFinalizingMerges; i++) {
    uint64_t beg = i * mergeFrame;
    mMerge(a, beg, beg + mergeFrame / 2, beg + mergeFrame);
}
```

## Optimierungsmöglichkeiten / Einschränkungen
- Bei der Version mit den [konventionellen Threads](#konventionelle-threads) wird ein Thread verschwendet - der Main Thread. Dieser ist jedoch zur veranschaulichung weggelassen. Man könnte den bestehenden Code folgendermassen noch erweitern:
```cpp
// Zeile 50
for (size_t i = 1; i < NO_OF_THREADS; i++) {
    threadpool[i] = std::thread(mSort, std::ref(a), i * parallelChunks, (i + 1) * parallelChunks);
}
std::thread(mSort, std::ref(a), 0, parallelChunks)

// ...

// Zeile 62
for (size_t i = 1; i < nFinalizingMerges; i++) {
    uint64_t beg = i * mergeFrame;
    threadpool[i] = std::thread(mMerge, std::ref(a), beg, beg + mergeFrame / 2, beg + mergeFrame);
}
std::thread(mSort, std::ref(a), 0, mergeFrame / 2, mergeFrame);
```

- Beide Versionen unterstützen nur Mengen, welche durch die Anzahl Threads geteilt werden können. Gibt man dem Programm eine Menge, welche nicht durch die Anzahl Threads teilbar ist, wird ein `assert`-Fehler geworfen. Als Lösung könnte man die Menge mit einer bekannten Zahl strecken, bis $|Menge| \mod n_{Threads} = 0$, und diese am Ende wieder entfernen.
