/* * TNI-EEG Artifact Detection Module
 * Logic for real-time flagging of EOG/EMG interference
 */

#define EEG_THRESHOLD_UV 150.0  // Microvolt threshold for blink detection
#define WINDOW_SIZE 64          // 250ms window at 256Hz
#define EMG_NOISE_VAR 50.0      // Variance threshold for muscle noise

typedef struct {
    uint32_t timestamp;
    float channel_data[30];
    bool is_artifact;
    uint8_t artifact_type; // 0: Clean, 1: Blink, 2: Muscle, 3: Motion
} EEG_Packet;

void process_buffer(float* raw_samples, EEG_Packet* output) {
    float sum = 0;
    float mean = 0;
    float variance = 0;

    // 1. Calculate Mean (for DC offset correction)
    for(int i = 0; i < WINDOW_SIZE; i++) {
        sum += raw_samples[i];
    }
    mean = sum / WINDOW_SIZE;

    // 2. Analyze Window for Artifacts
    for(int i = 0; i < WINDOW_SIZE; i++) {
        float corrected_val = raw_samples[i] - mean;
        
        // Threshold Check: Eye Blinks (EOG)
        if (abs(corrected_val) > EEG_THRESHOLD_UV) {
            output->is_artifact = true;
            output->artifact_type = 1; 
            break;
        }

        // Statistical Check: Muscle Noise (EMG)
        variance += pow(corrected_val - mean, 2);
    }
    
    variance /= WINDOW_SIZE;
    if (variance > EMG_NOISE_VAR) {
        output->is_artifact = true;
        output->artifact_type = 2;
    }

    // 3. Log to SD Card if valid, or log with flag
    log_to_sd_card(output);
}

