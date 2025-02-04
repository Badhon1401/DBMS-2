import numpy as np
import imageio.v2 as imageio  # Use imageio.v2 to avoid deprecation warnings
import cv2  # OpenCV for YCbCr conversion
import os
import multiprocessing as mp
from glob import glob
from tqdm import tqdm
import pickle

# Configuration
TRAIN_MASK_DIR = "Mask/"
TRAIN_IMAGE_DIR = "ibtd/"
TEST_IMAGE = "AMla.jpeg"
OUTPUT_IMAGE = "amla_final_1.jpeg"
MODEL_FILE = "skin_model1.pkl"  # File to save/load the trained model
ALPHA = 1.0  # Laplace smoothing factor

def train_worker(args):
    """Worker function to process a single training image-mask pair."""
    idx, mask_path, img_path = args
    try:
        mask = imageio.imread(mask_path)
        img = imageio.imread(img_path)
    except Exception as e:
        print(f"Error loading {mask_path} or {img_path}: {e}")
        return None

    if mask.shape != img.shape:
        print(f"Shape mismatch between {mask_path} and {img_path}")
        return None

    # Convert image to YCbCr color space
    img_ycbcr = cv2.cvtColor(img, cv2.COLOR_RGB2YCrCb)
    cr = img_ycbcr[:, :, 1]  # Cr channel
    cb = img_ycbcr[:, :, 2]  # Cb channel

    # Create skin mask (assuming white=non-skin in mask)
    skin_mask = np.any(mask < 250, axis=-1)

    # Separate skin and non-skin pixels
    skin_cr = cr[skin_mask]
    skin_cb = cb[skin_mask]
    non_skin_cr = cr[~skin_mask]
    non_skin_cb = cb[~skin_mask]

    # Compute histograms
    skin_cr_hist = np.histogram(skin_cr, bins=256, range=(0, 255))[0]
    skin_cb_hist = np.histogram(skin_cb, bins=256, range=(0, 255))[0]

    non_skin_cr_hist = np.histogram(non_skin_cr, bins=256, range=(0, 255))[0]
    non_skin_cb_hist = np.histogram(non_skin_cb, bins=256, range=(0, 255))[0]

    return skin_cr_hist, skin_cb_hist, non_skin_cr_hist, non_skin_cb_hist, len(skin_cr), len(non_skin_cr)

def train_model():
    """Trains the skin detection model using histogram probabilities."""
    mask_files = sorted(glob(os.path.join(TRAIN_MASK_DIR, "*.bmp")))
    img_files = [os.path.join(TRAIN_IMAGE_DIR, os.path.basename(f).replace(".bmp", ".jpg")) for f in mask_files]

    args = list(zip(range(len(mask_files)), mask_files, img_files))

    skin_cr = np.zeros(256)
    skin_cb = np.zeros(256)
    non_skin_cr = np.zeros(256)
    non_skin_cb = np.zeros(256)
    total_skin = 0
    total_non_skin = 0

    with mp.Pool() as pool:
        results = list(tqdm(pool.imap(train_worker, args), total=len(args), desc="Training"))

    for res in results:
        if res is None:
            continue
        scr, scb, nscr, nscb, ts, tns = res
        skin_cr += scr
        skin_cb += scb
        non_skin_cr += nscr
        non_skin_cb += nscb
        total_skin += ts
        total_non_skin += tns

    # Apply Laplace smoothing
    prob_skin_cr = (skin_cr + ALPHA) / (total_skin + ALPHA * 256)
    prob_skin_cb = (skin_cb + ALPHA) / (total_skin + ALPHA * 256)

    prob_non_skin_cr = (non_skin_cr + ALPHA) / (total_non_skin + ALPHA * 256)
    prob_non_skin_cb = (non_skin_cb + ALPHA) / (total_non_skin + ALPHA * 256)

    # Compute log ratios to avoid underflow
    log_ratio_cr = np.log(prob_skin_cr) - np.log(prob_non_skin_cr)
    log_ratio_cb = np.log(prob_skin_cb) - np.log(prob_non_skin_cb)

    # Apply sigmoid normalization instead of standardization
    log_ratio_cr = 1 / (1 + np.exp(-log_ratio_cr))
    log_ratio_cb = 1 / (1 + np.exp(-log_ratio_cb))

    # Save the trained model
    model = {
        'log_ratio_cr': log_ratio_cr,
        'log_ratio_cb': log_ratio_cb
    }
    with open(MODEL_FILE, 'wb') as f:
        pickle.dump(model, f)

    return log_ratio_cr, log_ratio_cb

# Correct the detection function
def detect_skin(image_path, log_ratio_cr, log_ratio_cb):
    """Detects skin regions in an image using the trained model."""
    img = imageio.imread(image_path)
    img_ycbcr = cv2.cvtColor(img, cv2.COLOR_RGB2YCrCb)

    cr = img_ycbcr[:, :, 1]  # Cr channel
    cb = img_ycbcr[:, :, 2]  # Cb channel

    # Compute log probability
    log_prob = log_ratio_cr[cr] + log_ratio_cb[cb]

    # Adaptive threshold using 80th percentile
    threshold = np.percentile(log_prob, 80)
    skin_mask = log_prob > threshold

    # Create output image (keep skin pixels, make non-skin white)
    result = np.full(img.shape, 255, dtype=np.uint8)
    result[skin_mask] = img[skin_mask]

    return result

# Correct the main block
if __name__ == "__main__":
    # Train the model or load from file if already trained
    if not os.path.exists(MODEL_FILE):
        print("Training model...")
        log_ratio_cr, log_ratio_cb = train_model()
    else:
        print("Loading trained model...")
        with open(MODEL_FILE, 'rb') as f:
            model = pickle.load(f)
        log_ratio_cr, log_ratio_cb = model['log_ratio_cr'], model['log_ratio_cb']
    
    # Detect skin (using correct parameters)
    result_img = detect_skin(TEST_IMAGE, log_ratio_cr, log_ratio_cb)
    
    # Save result
    imageio.imwrite(OUTPUT_IMAGE, result_img)
    print(f"Processing complete! Result saved to {OUTPUT_IMAGE}")