/*
 * system.cxx
 *
 *  Created on: Jul 22, 2018
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

#include <atomic>
#include <cstdlib>
#include <iostream>
#include <thread>

#include "conf.h"
#include "logging.h"
#include "system.h"

#include "system.unix.h"

namespace oemros {

using std::cout;
using std::cerr;
using std::endl;

using oemros_unix::os_setup_kill_timer;

static std::atomic<exit_code> system_fault_state = ATOMIC_VAR_INIT(exit_code::ok);

exit_code get_fault_state() {
    return system_fault_state.load(std::memory_order_seq_cst);
}

[[noreturn]] static void system_exit(exit_code code_in) {
    std::exit((int)code_in);
}

[[noreturn]] void exit_fault_state() {
    system_exit(system_fault_state.load(std::memory_order_seq_cst));
}

[[noreturn]] void system_panic__func(const char* file_in, int line_in, const char* function_in, const std::string& message_in) {
    cerr << endl << "OEMROS PANIC ";
    cerr << file_in << ":" << line_in << " " << function_in << endl;
    cerr << "  Message: " << message_in << endl;

    system_exit(exit_code::panic);
}

/*
 * System faulting logic
 *
 * The first time a fault happens the system status is set to indicate
 * the fault and all modules are told to run their fault handlers. Modules
 * have some fixed amount of seconds to complete their fault handlers and
 * after that time if any module has not stopped the process is going to
 * be exited anyway.
 *
 * If a fault happens while the system is in a faulted state that is a
 * double fault. A double fault will halt the thread that it happened in.
 * The other threads still have a chance at finishing their fault handlers
 * in the allowed time.
 *
 * If the fault handling grace period expires then oemros will panic
 *
 */

static exit_code get_system_fault_state() {
    return system_fault_state.load(std::memory_order_seq_cst);
}

[[noreturn]] static void handle_fault(const char* file_in, int line_in, const char* function_in, const std::string& message_in) {
    os_setup_kill_timer(DEFAULT_FAULT_GRACE_TIME);
    logjam::send_logevent(log_sources.oemros, logjam::loglevel::fatal, function_in, file_in, line_in, message_in);
    throw fault(file_in, line_in, function_in, message_in);
}

[[noreturn]] static void handle_double_fault(const char* file_in, int line_in, const char* function_in, const std::string& message_in) {
    auto known_state = get_system_fault_state();

    if (known_state != exit_code::doublefault) {
        if (! system_fault_state.compare_exchange_strong(known_state, exit_code::doublefault)) {
            // control can get here if the known_state was not doublefault and
            // it was updated to doublefault by another thread
            if (known_state != exit_code::doublefault) {
                system_panic("Failure when trying to set system_fault_state to doublefault");
            }
        }
    }

    logjam::send_logevent(log_sources.oemros, logjam::loglevel::fatal, function_in, file_in, line_in, message_in);
    throw double_fault(file_in, line_in, function_in, message_in.c_str());
}

[[noreturn]] void system_fault__func(const char* file_in, int line_in, const char* function_in, const std::string& message_in) {
    auto known_state = get_system_fault_state();

    if (known_state != exit_code::ok) {
        handle_double_fault(file_in, line_in, function_in, message_in);
        system_panic("handle_double_fault() returned");
    } else if (! system_fault_state.compare_exchange_strong(known_state, exit_code::fault)) {
        // if control reaches here the state changed between reading it
        // and trying to change it to being faulted which means
        // this is now a double fault
        assert(known_state != exit_code::ok);
        handle_double_fault(file_in, line_in, function_in, message_in);
        system_panic("handle_double_fault() returned");
    }

    handle_fault(file_in, line_in, function_in, message_in);
    system_panic("handle_fault() returned");
}

const char* enum_to_str(const exit_code& code_in) {
    switch(code_in) {
        case exit_code::ok: return "ok";
        case exit_code::fault: return "fault";
        case exit_code::doublefault: return "doublefault";
        case exit_code::panic: return "panic";
    }

    system_fault("could not find string for enum");
}

fault::fault(const char* file_in, int line_in, const char* function_in, const std::string& message_in)
: file(file_in), line(line_in), function(function_in), message(message_in) {
    assert(file != nullptr);
    assert(function_in != nullptr);
}

const char* fault::what() const noexcept {
    return message.c_str();
}

std::string errno_str(int errno_in) {
    std::string buf;
    buf += "errno_str() not yet implemented; errno = ";
    buf += errno_in;
    return buf;
}

}

