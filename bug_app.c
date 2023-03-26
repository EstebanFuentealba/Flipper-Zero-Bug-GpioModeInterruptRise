
#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>

// Screen is 128x64 px
static void app_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);
    canvas_draw_str_aligned(canvas, 128 / 2, 64 / 2, AlignCenter, AlignCenter, "Hola Mundo");
}
static void app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}
void input_clk_bug(void* ctx) {
    furi_assert(ctx);
    UNUSED(ctx);
}
int32_t bug_main(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Configure view port
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, app_draw_callback, view_port);
    view_port_input_callback_set(view_port, app_input_callback, event_queue);

    // Register view port in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    furi_hal_gpio_init(
        &gpio_ext_pc3,
        GpioModeInterruptRise,
        GpioPullNo,
        GpioSpeedVeryHigh); // <-- This line causes the "OK" to stop functioning when exiting the application, so a reboot of the Flipper Zero is required.
    furi_hal_gpio_add_int_callback(&gpio_ext_pc3, input_clk_bug, NULL);

    InputEvent event;

    bool running = true;
    while(running) {
        if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
            if((event.type == InputTypePress) || (event.type == InputTypeRepeat)) {
                switch(event.key) {
                default:
                    running = false;
                    break;
                }
            }
        }
        view_port_update(view_port);
    }
    //  Free Gpio
    //  Reset GPIO pins to default state
    furi_hal_gpio_init(&gpio_ext_pc3, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_remove_int_callback(&gpio_ext_pc3);

    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_GUI);

    return 0;
}
