CONTIKI=../../..
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\"

MODULES += core/net/llsec/adaptivesec
PROJECT_SOURCEFILES += util.c tiny_aes.c my_leds.c PZEM004T.c

CONTIKI_PROJECT = server

all: $(CONTIKI_PROJECT)

CONTIKI = ../../..
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include
