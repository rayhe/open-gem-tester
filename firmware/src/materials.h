/*
 * OpenGemTester - Materials Database
 *
 * Static database of gemstone and reference material signatures.
 * Each entry has a thermal conductivity range (W/mK) and an
 * electrical resistance range (ohms), plus a simplified
 * insulator/conductor flag.
 *
 * Identification works by scoring each candidate against the
 * measured thermal conductivity and resistance, then returning
 * the best match with a confidence value.
 */

#ifndef MATERIALS_H
#define MATERIALS_H

#include <Arduino.h>

struct MaterialSignature {
    const char* name;
    float thermalK_min;       // W/mK, lower bound of expected range
    float thermalK_center;    // W/mK, typical/nominal value
    float thermalK_max;       // W/mK, upper bound of expected range
    float resistance_min;     // ohms, lower bound
    float resistance_max;     // ohms, upper bound
    bool isInsulator;         // true = insulator, false = conductor/semiconductor
};

struct MaterialResult {
    const char* name;
    float confidence;         // 0.0 to 1.0
    int index;                // index into the materials database
};

/*
 * Material database.
 *
 * Thermal conductivity values from published literature.
 * Electrical resistance ranges are approximate for the probe geometry
 * (10M series resistor, small contact area, ~1mm probe tip).
 *
 * Notes on specific materials:
 *   Diamond (natural): k ~ 900-2500 W/mK depending on type and purity
 *   Moissanite (SiC): k ~ 350-600 W/mK, but also a semiconductor
 *   Corundum (sapphire, ruby): k ~ 25-50 W/mK
 *   The electrical test is critical for separating diamond from moissanite,
 *   since both have high thermal conductivity.
 */

static const MaterialSignature MATERIALS[] = {
    // Gemstones and simulants
    {
        "Diamond",
        900.0f, 2200.0f, 2500.0f,       // very high thermal conductivity
        1e9f, 1e12f,                      // strong insulator
        true
    },
    {
        "Moissanite",
        350.0f, 490.0f, 600.0f,          // high, but lower than diamond
        1e5f, 1e7f,                       // semiconductor (SiC)
        false
    },
    {
        "Sapphire",
        25.0f, 40.0f, 50.0f,             // corundum
        1e9f, 1e12f,                      // insulator
        true
    },
    {
        "Ruby",
        25.0f, 40.0f, 50.0f,             // corundum (same as sapphire)
        1e9f, 1e12f,                      // insulator
        true
    },
    {
        "Emerald",
        2.0f, 5.0f, 8.0f,               // beryl family, low k
        1e9f, 1e12f,                      // insulator
        true
    },
    {
        "Cubic Zirconia",
        1.5f, 2.0f, 3.0f,               // very low thermal conductivity
        1e9f, 1e12f,                      // insulator
        true
    },
    {
        "Glass",
        0.5f, 1.0f, 1.5f,               // soda-lime glass
        1e10f, 1e13f,                     // strong insulator
        true
    },
    {
        "Quartz",
        6.0f, 10.0f, 14.0f,             // natural crystalline quartz
        1e10f, 1e13f,                     // insulator
        true
    },
    {
        "Topaz",
        10.0f, 14.0f, 18.0f,            // aluminum silicate
        1e10f, 1e13f,                     // insulator
        true
    },
    {
        "Spinel",
        12.0f, 15.0f, 20.0f,            // magnesium aluminate
        1e9f, 1e12f,                      // insulator
        true
    },
    {
        "YAG",
        8.0f, 11.0f, 14.0f,             // yttrium aluminum garnet
        1e9f, 1e12f,                      // insulator
        true
    },
    // Metals (for sanity checking / error detection)
    {
        "Stainless Steel",
        12.0f, 16.0f, 25.0f,            // depends on alloy
        0.01f, 100.0f,                    // very low resistance
        false
    },
    {
        "Copper",
        350.0f, 400.0f, 420.0f,         // excellent thermal conductor
        0.001f, 1.0f,                     // near zero resistance
        false
    },
    {
        "Aluminum",
        200.0f, 235.0f, 250.0f,         // good thermal conductor
        0.001f, 1.0f,                     // near zero resistance
        false
    },
    // Calibration reference
    {
        "Air",
        0.02f, 0.026f, 0.03f,           // very low (gas)
        1e12f, 1e15f,                     // perfect insulator
        true
    },
};

static const int NUM_MATERIALS = sizeof(MATERIALS) / sizeof(MATERIALS[0]);

/*
 * Identify a material from measured thermal conductivity and resistance.
 *
 * Scoring algorithm:
 *   1. For each material, compute a thermal score based on how close
 *      the measured k is to the center value (Gaussian-like falloff).
 *   2. Compute an electrical score based on whether resistance falls
 *      within the expected range.
 *   3. Combine: total_score = thermal_weight * thermal_score + elec_weight * elec_score
 *   4. The material with the highest total score wins.
 *   5. Confidence is derived from the ratio of the best score to the second-best.
 *
 * The electrical test is weighted heavily when it can distinguish materials
 * with overlapping thermal ranges (e.g., diamond vs. moissanite).
 */
inline MaterialResult identifyMaterial(float thermalK, float resistance) {
    float scores[NUM_MATERIALS];
    float bestScore = -1.0f;
    float secondBest = -1.0f;
    int bestIdx = 0;

    for (int i = 0; i < NUM_MATERIALS; i++) {
        const MaterialSignature& mat = MATERIALS[i];

        // ---- Thermal score ----
        // Gaussian-like: score = exp(-(k - k_center)^2 / (2 * sigma^2))
        // sigma is proportional to the range width
        float kRange = mat.thermalK_max - mat.thermalK_min;
        float sigma = kRange * 0.5f;
        if (sigma < 1.0f) sigma = 1.0f;

        float kDiff = thermalK - mat.thermalK_center;
        float thermalScore = expf(-(kDiff * kDiff) / (2.0f * sigma * sigma));

        // Penalize if clearly outside the range
        if (thermalK < mat.thermalK_min * 0.5f || thermalK > mat.thermalK_max * 2.0f) {
            thermalScore *= 0.1f;
        }

        // ---- Electrical score ----
        float elecScore = 0.0f;

        if (resistance >= mat.resistance_min && resistance <= mat.resistance_max) {
            // Within expected range: full score
            elecScore = 1.0f;
        } else if (resistance < mat.resistance_min) {
            // More conductive than expected
            float ratio = mat.resistance_min / (resistance + 1.0f);
            elecScore = 1.0f / (1.0f + logf(ratio + 1.0f));
        } else {
            // More resistive than expected
            float ratio = resistance / (mat.resistance_max + 1.0f);
            elecScore = 1.0f / (1.0f + logf(ratio + 1.0f));
        }

        // ---- Combined score ----
        // Weight electrical more heavily when it can differentiate
        // (e.g., diamond vs moissanite both have high k but differ electrically)
        float thermalWeight = 0.55f;
        float elecWeight = 0.45f;

        scores[i] = thermalWeight * thermalScore + elecWeight * elecScore;

        if (scores[i] > bestScore) {
            secondBest = bestScore;
            bestScore = scores[i];
            bestIdx = i;
        } else if (scores[i] > secondBest) {
            secondBest = scores[i];
        }
    }

    // Compute confidence: how much better is the best vs second-best?
    float confidence = 0.0f;
    if (bestScore > 0.0f) {
        if (secondBest <= 0.0f) {
            confidence = 1.0f;
        } else {
            // Ratio-based confidence: if best is 2x second, confidence ~1.0
            float ratio = bestScore / secondBest;
            confidence = 1.0f - (1.0f / ratio);
            if (confidence < 0.0f) confidence = 0.0f;
            if (confidence > 1.0f) confidence = 1.0f;
        }
        // Also factor in absolute score quality
        confidence *= bestScore;
    }

    MaterialResult result;
    result.name = MATERIALS[bestIdx].name;
    result.confidence = confidence;
    result.index = bestIdx;

    return result;
}

#endif // MATERIALS_H
