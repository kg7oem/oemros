/*
 * hamlib.h
 *
 *  Created on: Jul 31, 2018
 *      Author: tyler
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include <utility>

#include "object.h"
#include "radio.h"
#include "system.h"

namespace hamlib {

#include <hamlib/rig.h>

}

namespace oemros {

void hamlib_bootstrap();

struct hamlib_error : public exception {
    const int error_num;
    hamlib_error(int error_num_in);
};

template <typename T> struct hamlib_result {
    using value_type = T;

    const int error;
    const T value;

    hamlib_result(const int& error_in, const T& value_in)
    : error(error_in), value(value_in) { }
    operator bool() { return error == hamlib::RIG_OK; }
    operator T() {
        if (! *this) throw hamlib_error(error);
        return value;
    }
    std::string error_str() {
        return std::string(hamlib::rigerror(error));
    }
};

template <class T> hamlib_result<T>
make_hamlib_result(const int& error_in, const T& value_in) {
    return hamlib_result<T>(error_in, value_in);
}

class hamlib_rig : public baseobj {
    public:
        using freq_type = hamlib::freq_t;
        using vfo_type = hamlib::vfo_t;

    private:
        hamlib::RIG* hl_rig = nullptr;

    public:
        const hamlib::rig_model_t model;
        hamlib_rig(const hamlib::rig_model_t& model_in) : model(model_in) { }
        ~hamlib_rig();
        bool open();
        hamlib_result<float> get_alc(vfo_type vfo_in = RIG_VFO_CURR);
        hamlib_result<freq_type> get_freq(vfo_type vfo_in = RIG_VFO_CURR);
        hamlib_result<int> get_strength(vfo_type vfo_in = RIG_VFO_CURR);
        hamlib_result<float> get_swr(vfo_type vfo_in = RIG_VFO_CURR);
};

class hamlib_radio : public radio {
    private:
        hamlib_rig* rig;

    protected:
        virtual void update__alc() override;
        virtual void update__power() override;
        virtual void update__swr() override;
        virtual void update__tuner() override;

    public:
        hamlib_radio(const hamlib::rig_model_t& model_in);
        ~hamlib_radio();
        bool open();
};

}
