 
# Ardoscope

Осциллограф на основе Arduino

Для компиляции необходим модуль serialport:

    >sudo apt-get install libqt5serialport5-dev


Компиляция:

    >qmake
    >make

Управление мышью:
* Двойной щелчёк на дисплее масштабирует окно по сигналу.
* Колесо прокрутки мыши (+- включенный NumLock) сдвигает сигнал на дисплее или выбирает диапазон по U и T.
