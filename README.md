Program, który napisałem, działa w następujący sposób:  
Najpierw spośród wprowadzonych punktów wyznacza 4 punkty skrajne – z największym X, z najmniejszym X, z największym Y i z najmniejszym Y. Te punkty odrazu zaznacza jako należące do zagrody. Zagroda w tym momencie jest czworokątem (oprócz specjalnych przypadków).  
Tworzy z tych punktów 4 prostokąty – z górnego i prawego tworzy prawy górny prostokąt i tak dalej tworzy prawy dolny, lewy górny i lewy dolny.  
Pozostałe punkty przydziela do ćwiartek, w których się znajdują. Robi to tak: jeśli punkt leży w danym prostokącie i dodatkowo leży poza obecną zagrodą, to jest on dodawany do tablicy danego prostokąta. Sprawdzanie czy leży poza zagrodą polega na takim sprawdzeniu: jeśli np. punkt jest w prawym górnym prostokącie to leży poza zagrodą jeśli leży nad linią łączącą skrajnie górny i skrajnie prawy punkt. Jeden punkt w skrajnych przypadkach może się znaleźć w więcej niż jednej tablicy.  
Dalej, dla każdej ćwiartki oddzielnie odbywa się dodawanie punktów do zagrody:  
(Pracując na przykładnie prawej górnej ćwiartki)  
Punkty w tablicy tej ćwiartki są sortowane w kolejności od największego Y do najmniejszego (od góry do dołu).  
Tworzona jest połączona lista, w której narazie znajduje się punkt skrajnie górny i skrajnie prawy.  
Ustalana jest zmienna trzymająca wartość najbardziej prawego X do tej pory przyłączonego do zagrody punktu.  
Dla każdego punktu w tablicy:  
1. Jeśli punkt leży na lewo od najbardziej prawego X to wiadomo, że nie jest on częścią zagrody (chyba że leży na tym samym Y co punkt skrajnie górny – wtedy jest oznaczany jako część zagrody).  
2. Jeśli punkt leży dokładnie na X najbardziej prawego X to wiadomo, że nie jest częścią zagrody (chyba że ten najbardziej prawy X jest równy Xowi skrajnie prawego punktu – wtedy jest dodawany do listy i oznaczany jako część zagrody).  
3. Jeśli punkt leży na prawo od najbardziej prawego X to jest dodawany do listy i oznaczany jako część zagrody.  
Po przypadku 2 i 3 przeprowadzane jest usuwanie wklęsłości:  
Jeśli poprzedni punkt w liście tworzy kąt wklęsający do środka zagrody między obecnym punktem i punktem 2 miejsca wstecz, to jest usuwany z listy i oznaczany jako niebędący częścią zagrody (wiadomo, że kąt jest wklęsły, jeśli punkt poprzedni leży POD linią między punktem obecnym I punktem 2 wstecz). Jeśli został usunięty jakiś punkt to ta operacja jest powtarzana o jedno miejsce dalej (między punktem obecnym, 2 wstecz i 3 wstecz). Usuwanie kończy się po napotkaniu pierwszej wypukłości, lub gdy punkt dalej wstecz jest punktem skrajnie górnym (co jest równoważne temu, że w liście są mniej niż 4 punkty).

Po sprawdzeniu wszystkich punktów z tablicy przeprowadzane jest dodatkowe usuwanie wklęsłości zaczynając od punktu skrajnie prawego.

Dla pozostałych ćwiartek operacje są wykonywane symetrycznie: dla lewych ćwiartek pilnowany jest najbardziej LEWY X, dla dolnych ćwiartek sortowanie układa punkty od dołu do góry, a wklęsłe punkty leżą NAD linią między sąsiadami, itd.

Po tym wszystkim wypisywane są wszystkie punkty które zostały oznaczone jako część zagrody.

Mówiąc w skrócie, można powiedzieć, że ten program dla każdej ćwiartki dodaje od góry (lub odpowiednio od dołu) punkty, które nie leżą w obecnej zagrodzie i na bieżąco usuwa punkty tworzące wklęsłości.

W tym programie zastosowałem techniki takie jak:  
Listy połączone zamiast wektorów w miejscach, gdzie odbywa się dodawanie i usuwanie elementów.  
Wymienione tablice i listy tak naprawdę pracują na wskaźnikach do punktów zamiast na punktach, żeby nie trzeba było potem niewiadomo jak łączyć ze sobą kilku różnych zbiorów i usuwać duplikaty. Pomaga to także uniknąć niepotrzebnego kopiowania danych.  
Skorzystałem z gotowego algorytmu sortującego w nagłówku \<algorithms\>.
