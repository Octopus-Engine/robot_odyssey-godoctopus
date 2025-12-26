#include "VatLibrary.h"

namespace godot {

void VatLibrary::_notification(int p_notification)
{
	switch (p_notification) {
		case NOTIFICATION_PROCESS: {
			//_process(get_process_delta_time());
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			//_physics_process(get_physics_process_delta_time());
		} break;
		case NOTIFICATION_READY: {
			//_ready();
			set_process(true);
			set_physics_process(true);
		} break;
	}
}

}
