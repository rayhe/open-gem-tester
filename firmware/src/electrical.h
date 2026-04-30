/*
 * OpenGemTester - Electrical Probe Header
 *
 * Measures electrical conductivity of a stone to distinguish
 * diamond (insulator) from moissanite (semiconductor) and metals (conductor).
 * Uses a simple voltage divider: 3.3V -- 10M series resistor -- probe tip -- stone -- GND plate.
 */

#ifndef ELECTRICAL_H
#define ELECTRICAL_H

#include <Arduino.h>
#include <Adafruit_ADS1X15.h>

/* Classification of electrical conductivity */
enum ConductivityClass {
    COND_INSULATOR,      // >1G ohm (diamond, glass, CZ, most gems)
    COND_SEMICONDUCTOR,  // 100K - 1G ohm (moissanite, some minerals)
    COND_CONDUCTOR       // <100K ohm (metals)
};

class ElectricalProbe {
public:
    ElectricalProbe();

    /*
     * Initialize the electrical probe subsystem.
     * ads             - pointer to ADS1115 (shared)
     * switchPin       - GPIO controlling the electrical mode MOSFET
     * seriesResistance - series resistor value in ohms (10M)
     */
    void init(Adafruit_ADS1115* ads, int switchPin, float seriesResistance);

    /* Enable electrical measurement mode (turn on MOSFET to connect circuit). */
    void enableElectricalMode();

    /* Disable electrical mode (disconnect circuit, return to thermal mode). */
    void disableElectricalMode();

    /*
     * Take a single resistance measurement.
     * Reads ADS1115 AIN1, calculates stone resistance from the voltage divider.
     * Returns resistance in ohms.
     */
    float measureResistance();

    /*
     * Take multiple readings and return the median value.
     * Outlier rejection: discards readings outside 2x the interquartile range.
     * Returns median resistance in ohms.
     */
    float averageResistance(int samples);

    /*
     * Classify the measured resistance into insulator, semiconductor, or conductor.
     */
    ConductivityClass classifyConductivity(float resistance);

private:
    Adafruit_ADS1115* _ads;
    int _switchPin;
    float _seriesR;

    /* Read voltage from ADS1115 channel 1 (AIN1). */
    float readVoltage();

    /* Simple insertion sort for small arrays (used in median calculation). */
    static void sortFloats(float* arr, int count);
};

#endif // ELECTRICAL_H
