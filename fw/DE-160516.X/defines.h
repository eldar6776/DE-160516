/******************************************************************************
 *
 *               Apsolutni enkoder ugla za ILLIG RDM 58/3
 *
 *                      Makro definicije
 *
 *******************************************************************************
 * Ime fajla:       defines.h
 *
 * Procesor:        PIC18F23K20
 *
 * Kompajler:       MCC18 v3.47
 *
 * IDE:             MPLAB X IDE v3.26 (java 1.8.0_65)
 *
 * Firma:           PROVEN d.o.o. Sarajevo
 *
 * Autor:           <mailto> eldar6776@hotmail.com
 *
 * Prava:           Sva prava na programski kod zadrzava firma
 *                  PROVEN d.o.o. Sarajevo.
 *                  <mailto> proven@bih.net.ba
 *                  Adema Buce 102, 71000 Sarajevo
 *                  Federacija Bosne i Hercegovine
 *                  Bosna i Hercegovina
 *                  tel.:+387 33 651 407
 *                  tel.:+387 33 718 670
 *                  fax.:+387 33 718 671
 *                  www.proven.com.ba
 *
 *****************************************************************************/
//
//
//
#ifndef DEFINES_H
#define	DEFINES_H
//
//
//
#include <delays.h>
#include "defines.h"

#define ClockSSI()      (SSI_CLK = HIGH, Delay10TCY(),SSI_CLK = LOW, Delay10TCY())

#endif	/* DEFINES_H */

