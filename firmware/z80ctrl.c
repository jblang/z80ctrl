#include "util/pgmspace.h"

#include "cli/cli.h"

#include "hardware/hardware.h"
#include "hardware/term.h"

#include "util/ffwrap.h"

/**
 * z80ctrl entry point
 */
int main(void)
{
    term_init();
    printf_P(PSTR(
        "\n      ___   ___       _        _ "
        "\n  ___( _ ) / _ \\  ___| |_ _ __| |"
        "\n |_  / _ \\| | | |/ __| __| '__| |"
        "\n  / / (_) | |_| | (__| |_| |  | |"
        "\n /___\\___/ \\___/ \\___|\\__|_|  |_|"
        "\n by J.B. Langston "
#ifdef GITVERSION
        "(" GITVERSION ")"
#endif
        "\n"));
    hardware_init();
    ffw_init();
    cli_start();
}