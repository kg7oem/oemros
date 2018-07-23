/*
 * system.unix.cxx
 *
 *  Created on: Jul 25, 2018
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

#include "system.h"
#include "system.unix.h"

namespace oemros_unix {

#include <cassert>
#include <signal.h>
#include <unistd.h>

static void kill_timer_alarm_handler(UNUSED int signum_in) {
    assert(signum_in == SIGALRM);
    system_panic("grace period for fault handlers has expired");
}

void os_setup_kill_timer(unsigned int seconds_in) {
    auto old_handler = signal(SIGALRM, kill_timer_alarm_handler);

    if (old_handler == SIG_ERR) {
        system_panic("could not set handler for SIGALRM: ", oemros::errno_str(errno));
    }

    assert(old_handler == SIG_DFL);
    assert(seconds_in > 0);
    auto alarm_result = alarm(seconds_in);
    assert(alarm_result == 0);
}

}



