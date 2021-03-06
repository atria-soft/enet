/** @file
 * @author Edouard DUPIN
 * @copyright 2014, Edouard DUPIN, all right reserved
 * @license MPL v2.0 (see license file)
 */

#include <enet/debug.hpp>

int32_t enet::getLogId() {
	static int32_t g_val = elog::registerInstance("enet");
	return g_val;
}
