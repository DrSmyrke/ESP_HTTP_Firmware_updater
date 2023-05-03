FILE = myfunctions.cpp
!exists($$FILE):system(wget https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/$$FILE -O $$FILE)
SOURCES += $$FILE

FILE = myfunctions.h
!exists($$FILE):system(wget https://raw.githubusercontent.com/DrSmyrke/QT-Libs/master/$$FILE -O $$FILE)
HEADERS += $$FILE



