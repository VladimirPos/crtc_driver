crtc_driver эмулирует работу rtc в 3 режимах -- стандартный, ускоренный, замедленный. Стандартный режим доступен при запуске модуля. Для установки ускоренного режима необходимо передать параметр 1 при установке модуля, либо воспользоваться интерфейсом /proc, используя команду echo 1 > /proc/crtc (для замедленного режима используйте параметр 0). При инициализации с помощью insmod, модуль встраивает устройство в /dev по динамически получаемому major-номеру. При инициализации модуль установит системное время  в качестве времени устройства rtc. У модуля имеется возможность установить собственное значение времени с помощью команды вида echo "YYYY-MM-DD hh-mm-ss.nnnnnn" > /dev/crtc  Для чтения значения устройства используйте cat /dev/crtc Для выгрузки модуля используйте команду rmmod crtc.ko