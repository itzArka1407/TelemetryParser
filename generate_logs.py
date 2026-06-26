import time
import random
from datetime import datetime

# Exact matching tokens with fixed-width hyphen layout
LOG_TYPES = ["INFO-", "WARN-", "ERROR", "OTHER"]

SAMPLE_MESSAGES = {
    "INFO-": [
        "Database connection pooling established successfully.",
        "User session token refreshed for uid 89123.",
        "Static asset cache flushed completely in 4ms.",
        "Inbound API request processed on route /v1/metrics."
    ],
    "WARN-": [
        "Disk utilization has crossed 85% threshold on /dev/nvme0n1p3.",
        "Slow query detected: SELECT * FROM users took 243ms.",
        "API Gateway dropped connection pool connection due to timeout.",
        "Rate limit bucket saturated for host address 192.168.1.45."
    ],
    "ERROR": [
        "CRITICAL: Failed to write frame block to ScyllaDB cluster node.",
        "Segmentation fault intercepted during SIMD batch vector parsing.",
        "TLS Handshake failed: Client certificate has expired.",
        "Connection refused: Socket allocator out of file descriptors."
    ],
    "OTHER": [
        "DEBUG: Instruction pipeline alignment check passed.",
        "TRACE: Entering stack frame routine live_media_relay().",
        "DEBUG: Zero-copy packet serialization boundary verified.",
        "TRACE: Allocator blocks assigned: initial block group size 4096."
    ]
}

def generate_log_line():
    # 1. Enforce random weight distribution (lots of INFO, fewer ERRORs)
    log_type = random.choices(LOG_TYPES, weights=[65, 20, 5, 10], k=1)[0]
    
    # 2. Get exact timestamp matching your profile structure
    timestamp = datetime.now().strftime("%d:%m:%Y %H:%M:%S")
    message = random.choice(SAMPLE_MESSAGES[log_type])
    
    # 3. Construct the clean structural output line
    return f"[{timestamp} {log_type}] {message}\n"

def main():
    log_filename = "test_logs.txt"
    print(f"[*] Creating initial log bank in {log_filename}...")
    
    # Pre-populate the file with 500 lines so the application has initial historical data
    with open(log_filename, "w") as f:
        for _ in range(500):
            f.write(generate_log_line())
            
    print(f"[*] Initial bank written. Now entering active live stream mode (Press Ctrl+C to stop)...")
    
    # Infinite tail simulation append engine
    try:
        while True:
            with open(log_filename, "a") as f:
                line = generate_log_line()
                f.write(line)
                f.flush() # Force kernel space flush down to the disk sector immediately
            
            # Sleep a completely random fraction of a second to simulate active system chaos
            time.sleep(random.uniform(0.05, 0.4))
    except KeyboardInterrupt:
        print("\n[*] Log streaming simulation cleanly terminated.")

if __name__ == "__main__":
    main()
