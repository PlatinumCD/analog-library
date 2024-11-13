
#ifndef ANALOG_TYPE_H
#define ANALOG_TYPE_H

class AnalogType {
public:
    AnalogType() : scale_factor(1.0f) {}

    void update_scale_factor(double scale) {
        scale_factor *= scale;
    }

    double get_scale_factor() {
        return scale_factor;
    }

    double scale_factor;
};


#endif
