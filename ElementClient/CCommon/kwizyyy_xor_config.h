#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * XOR unpacking for elementdata / task packs (elements.data, tasks.data, etc.).
 *
 * Plain (unencrypted) .data files — pick ONE approach:
 *   1) Preprocessor: define KWIZYYY_READ_PLAIN_DATA_FILES in the project (dev tools).
 *      Remove the macro for release builds that must read XOR-packed files.
 *   2) Runtime (Windows): with developer unlock (see below), set in Configs\element_data_xor.ini:
 *        [DataXor]
 *        enabled=0
 *      Then buffers are copied as-is (no XOR).
 *
 * XOR key / default "TROUXA" when XOR is on:
 *   ini (custom key, enabled=1) applies only if developer unlock is on:
 *   - define ALLOW_ELEMENT_XOR_CONFIG in the project, or
 *   - place Configs\element_data_xor.dev next to the game (empty file is enough).
 *   Public release: do not ship .dev and do not define ALLOW_ELEMENT_XOR_CONFIG — ini is ignored.
 */
void KwizyyyXorApplyToBuffer(char* out, const char* in, size_t len);

#ifdef __cplusplus
}
#endif
