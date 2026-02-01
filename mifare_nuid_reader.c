#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <notification/notification_messages.h>
#include <lib/nfc/nfc_device.h>
#include <lib/nfc/protocols/mf_classic/mf_classic.h>
#include <lib/nfc/nfc_poller.h>
#include <lib/nfc/protocols/mf_classic/mf_classic_poller.h>

typedef enum {
    EventTypeTick,
    EventTypeKey,
} EventType;

typedef struct {
    EventType type;
    InputEvent input;
} AppEvent;

typedef struct {
    Gui* gui;
    ViewPort* view_port;
    FuriMessageQueue* event_queue;
    NotificationApp* notifications;
    Nfc* nfc;
    NfcPoller* poller;
    bool card_detected;
    uint8_t uid[10];
    uint8_t uid_len;
    char uid_str[32];
} MifareNuidApp;

static void render_callback(Canvas* canvas, void* ctx) {
    MifareNuidApp* app = ctx;
    
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 12, "Mifare NUID Reader");
    
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 26, "Place card near reader");
    
    if(app->card_detected) {
        canvas_draw_str(canvas, 2, 40, "UID:");
        canvas_draw_str(canvas, 2, 52, app->uid_str);
    } else {
        canvas_draw_str(canvas, 2, 40, "Waiting for card...");
    }
    
    canvas_draw_str(canvas, 2, 62, "Press Back to exit");
}

static void input_callback(InputEvent* input_event, void* ctx) {
    MifareNuidApp* app = ctx;
    AppEvent event = {.type = EventTypeKey, .input = *input_event};
    furi_message_queue_put(app->event_queue, &event, FuriWaitForever);
}

static NfcCommand poller_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    MifareNuidApp* app = context;
    
    NfcEvent* nfc_event = event;
    if(nfc_event->type == NfcEventTypePollerReady) {
        const NfcDeviceData* data = nfc_poller_get_data(app->poller);
        
        if(data) {
            // Get UID from the NFC data
            app->uid_len = data->uid_len;
            memcpy(app->uid, data->uid, app->uid_len);
            
            // Convert UID to hex string
            app->uid_str[0] = '\0';
            for(uint8_t i = 0; i < app->uid_len; i++) {
                char byte_str[4];
                snprintf(byte_str, sizeof(byte_str), "%02X", app->uid[i]);
                strcat(app->uid_str, byte_str);
                if(i < app->uid_len - 1) {
                    strcat(app->uid_str, " ");
                }
            }
            
            app->card_detected = true;
            
            // Vibrate to indicate card detected
            notification_message(app->notifications, &sequence_success);
            
            view_port_update(app->view_port);
        }
    }
    
    return NfcCommandContinue;
}

static MifareNuidApp* app_alloc() {
    MifareNuidApp* app = malloc(sizeof(MifareNuidApp));
    
    app->event_queue = furi_message_queue_alloc(8, sizeof(AppEvent));
    app->view_port = view_port_alloc();
    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    
    app->card_detected = false;
    app->uid_len = 0;
    memset(app->uid, 0, sizeof(app->uid));
    strcpy(app->uid_str, "");
    
    view_port_draw_callback_set(app->view_port, render_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app);
    
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);
    
    // Initialize NFC
    app->nfc = nfc_alloc();
    
    return app;
}

static void app_free(MifareNuidApp* app) {
    if(app->poller) {
        nfc_poller_stop(app->poller);
        nfc_poller_free(app->poller);
    }
    
    nfc_free(app->nfc);
    
    gui_remove_view_port(app->gui, app->view_port);
    view_port_free(app->view_port);
    
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(app->event_queue);
    
    free(app);
}

int32_t mifare_nuid_reader_app(void* p) {
    UNUSED(p);
    
    MifareNuidApp* app = app_alloc();
    
    // Start NFC poller for Mifare Classic
    app->poller = nfc_poller_alloc(app->nfc, NfcProtocolMfClassic);
    nfc_poller_start(app->poller, poller_callback, app);
    
    AppEvent event;
    bool running = true;
    
    while(running) {
        if(furi_message_queue_get(app->event_queue, &event, 100) == FuriStatusOk) {
            if(event.type == EventTypeKey) {
                if(event.input.key == InputKeyBack && event.input.type == InputTypeShort) {
                    running = false;
                }
            }
        }
        
        view_port_update(app->view_port);
    }
    
    app_free(app);
    
    return 0;
}
