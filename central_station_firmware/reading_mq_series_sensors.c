

// This method probably works for other MQ-x sensors as well.
float read_co_mq7() {
    // uint16_t range = MQ_7_UPPER_LIMIT - MQ_7_LOWER_LIMIT;
    // return (BOARD_ADC_RESOLUTION / range) * adc_read() + MQ_7_LOWER_LIMIT;

    // const float conv_factor = 3.3f / (1 << 12);
    // float divider_gain = 1.5;
    // float r0 = 70;
    // float rl = 10000;

    // uint16_t raw = adc_read();
    // float v_out = raw * conv_factor;          // Voltage at ADC pin
    // float v_sensor = v_out * divider_gain;    // Undo divider → sensor voltage

    // // Calculate Rs
    // float rs = rl * (5.0 - v_sensor) / v_sensor;

    // // Normalize against baseline R0
    // float ratio = rs / r0;

    // //printf("Raw: %u, Vout: %.3f V, Vsensor: %.3f V, Rs: %.1f Ω, Rs/R0: %.3f\n",
    // //        raw, v_out, v_sensor, rs, ratio);

    // float rs_ro = rs / r0;

    // // coefficients from datasheet fit
    // const float m = -1.53f;    // slope
    // const float b = 2.30f;     // intercept

    // float log_ppm = m * log10f(rs_ro) + b;
    // return powf(10, log_ppm);

    // READ R0
    // adc_select_input(0);
    // float sensor_volt; //Define variable for sensor voltage
    // float RS_air; //Define variable for sensor resistance
    // float R0; //Define variable for R0
    // float sensorValue; //Define variable for analog readings
    // for(int x = 0 ; x < 500 ; x++) //Start for loop
    // {
    //     sensorValue = sensorValue + adc_read(); //Add analog values of sensor 500 times
    // }
    // sensorValue = sensorValue/500.0; //Take average of readings
    // printf("Senor value: %f\n", sensorValue);
    // sensor_volt = sensorValue * (5.0/4096.0); //Convert average to voltage 
    // float RL = 10000.0; // 10k load resistor
    // RS_air = (5.0 - sensor_volt) * RL / sensor_volt;
    // R0 = RS_air/4.4; //Calculate R0


    adc_select_input(0);

    float sensorValue = 0;
    for (int x = 0; x < 500; x++) {
        sensorValue += adc_read();
    }
    sensorValue /= 500.0f;  // average ADC reading

    // Convert ADC reading to voltage (Pico ADC ref = 3.3 V)
    float v_out = sensorValue * (3.3f / 4096.0f);

    // Undo the module’s voltage divider (≈1.5×)
    float v_sensor = v_out * 1.5f;

    // Calculate Rs
    float RL = 10000.0f; // 10k load resistor
    float RS_air = (5.0f - v_sensor) * RL / v_sensor;

    // Calculate R0
    float R0 = 9100;
    //float R0 = RS_air / 4.4f;

    printf("ADC avg: %.1f, Vout: %.3f V, Vsensor: %.3f V, Rs: %f Ω, R0: %f Ω\n",
        sensorValue, v_out, v_sensor, RS_air, R0);

    float ratio = RS_air / R0;

    // Line fit from datasheet log-log curve
    const float m = -1.66f;
    const float b = 0.68f;

    float log_ppm = m * log10f(ratio) + b;
    float ppm = powf(10, log_ppm);

    return ppm;
}