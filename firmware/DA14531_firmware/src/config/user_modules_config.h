#pragma once

/***************************************************************************************/
/* Exclude or not a module in user's application code.                                 */
/*                                                                                     */
/* (0) - The module is included. The module's messages are handled by the SDK.         */
/*                                                                                     */
/* (1) - The module is excluded. The user must handle the module's messages.           */
/*                                                                                     */
/* Note:                                                                               */
/*      This setting has no effect if the respective module is a BLE Profile           */
/*      that is not used included in the user's application.                           */
/***************************************************************************************/

// THESE MODULES ARE INCLUDED

#define EXCLUDE_DLG_GAP             (0)
#define EXCLUDE_DLG_TIMER           (0)
#define EXCLUDE_DLG_SEC             (0)
#define EXCLUDE_DLG_CUSTS1          (0)
#define EXCLUDE_DLG_DISS            (0)
#define EXCLUDE_DLG_CUSTS2          (0)

// THESE MODULES ARE NOT INCLUDED, THAT IS TO SAY THEY ARE EXCLUDED

#define EXCLUDE_DLG_MSG             (1)
#define EXCLUDE_DLG_PROXR           (1)
#define EXCLUDE_DLG_BASS            (1)
#define EXCLUDE_DLG_FINDL           (1)
#define EXCLUDE_DLG_FINDT           (1)
#define EXCLUDE_DLG_SUOTAR          (1)
