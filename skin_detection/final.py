import numpy as np
import imageio.v2 as imageio  # Use imageio.v2 to avoid deprecation warnings
import os
import multiprocessing as mp
from glob import glob
from tqdm import tqdm
import pickle

# Configuration
TRAIN_MASK_DIR = "Mask/"
TRAIN_IMAGE_DIR = "ibtd/"
TEST_IMAGE = "AMla.jpeg"
OUTPUT_IMAGE = "AMla_fina_res.jpeg"
MODEL_FILE = "skin_model.pkl"  # File to save/load the trained model
THRESHOLD = 0.0  # Start with 0.0 and adjust based on results
ALPHA = 1.0  # Laplace smoothing factor

def train_worker(args):
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

    # Create skin mask (assuming white=non-skin in mask)
    skin_mask = np.any(mask < 250, axis=-1)
    
    skin_pixels = img[skin_mask]
    non_skin_pixels = img[~skin_mask]

    skin_r = np.histogram(skin_pixels[:, 0], bins=256, range=(0, 255))[0]
    skin_g = np.histogram(skin_pixels[:, 1], bins=256, range=(0, 255))[0]
    skin_b = np.histogram(skin_pixels[:, 2], bins=256, range=(0, 255))[0]

    non_skin_r = np.histogram(non_skin_pixels[:, 0], bins=256, range=(0, 255))[0]
    non_skin_g = np.histogram(non_skin_pixels[:, 1], bins=256, range=(0, 255))[0]
    non_skin_b = np.histogram(non_skin_pixels[:, 2], bins=256, range=(0, 255))[0]

    return (skin_r, skin_g, skin_b, non_skin_r, non_skin_g, non_skin_b,
            len(skin_pixels), len(non_skin_pixels))

def train_model():
    mask_files = sorted(glob(os.path.join(TRAIN_MASK_DIR, "*.bmp")))
    img_files = [os.path.join(TRAIN_IMAGE_DIR, os.path.basename(f).replace(".bmp", ".jpg")) for f in mask_files]
    
    args = list(zip(range(len(mask_files)), mask_files, img_files))
    
    skin_r = np.zeros(256)
    skin_g = np.zeros(256)
    skin_b = np.zeros(256)
    non_skin_r = np.zeros(256)
    non_skin_g = np.zeros(256)
    non_skin_b = np.zeros(256)
    total_skin = 0
    total_non_skin = 0

    with mp.Pool() as pool:
        results = list(tqdm(pool.imap(train_worker, args), total=len(args), desc="Training"))
    
    for res in results:
        if res is None:
            continue
        sr, sg, sb, nsr, nsg, nsb, ts, tns = res
        skin_r += sr
        skin_g += sg
        skin_b += sb
        non_skin_r += nsr
        non_skin_g += nsg
        non_skin_b += nsb
        total_skin += ts
        total_non_skin += tns

    # Apply Laplace smoothing
    prob_skin_r = (skin_r + ALPHA) / (total_skin + ALPHA * 256)
    prob_skin_g = (skin_g + ALPHA) / (total_skin + ALPHA * 256)
    prob_skin_b = (skin_b + ALPHA) / (total_skin + ALPHA * 256)

    prob_non_skin_r = (non_skin_r + ALPHA) / (total_non_skin + ALPHA * 256)
    prob_non_skin_g = (non_skin_g + ALPHA) / (total_non_skin + ALPHA * 256)
    prob_non_skin_b = (non_skin_b + ALPHA) / (total_non_skin + ALPHA * 256)

    # Calculate log ratios to avoid underflow
    log_ratio_r = np.log(prob_skin_r) - np.log(prob_non_skin_r)
    log_ratio_g = np.log(prob_skin_g) - np.log(prob_non_skin_g)
    log_ratio_b = np.log(prob_skin_b) - np.log(prob_non_skin_b)

    # Normalize log ratios to improve thresholding
    log_ratio_r = (log_ratio_r - np.mean(log_ratio_r)) / np.std(log_ratio_r)
    log_ratio_g = (log_ratio_g - np.mean(log_ratio_g)) / np.std(log_ratio_g)
    log_ratio_b = (log_ratio_b - np.mean(log_ratio_b)) / np.std(log_ratio_b)

    # Save the trained model
    model = {
        'log_ratio_r': log_ratio_r,
        'log_ratio_g': log_ratio_g,
        'log_ratio_b': log_ratio_b,
        'threshold': THRESHOLD
    }
    with open(MODEL_FILE, 'wb') as f:
        pickle.dump(model, f)

    return log_ratio_r, log_ratio_g, log_ratio_b

def detect_skin(image_path, log_ratio_r, log_ratio_g, log_ratio_b):
    img = imageio.imread(image_path)
    r = img[:, :, 0]
    g = img[:, :, 1]
    b = img[:, :, 2]
    
    # Vectorized probability calculation
    log_prob = log_ratio_r[r] + log_ratio_g[g] + log_ratio_b[b]
    
    # Create mask based on threshold
    skin_mask = log_prob > THRESHOLD
    
    # Create output image
    result = np.full(img.shape, 255, dtype=np.uint8)
    result[skin_mask] = img[skin_mask]
    
    return result

if __name__ == "__main__":
    # Train the model or load from file if already trained
    if not os.path.exists(MODEL_FILE):
        print("Training model...")
        lr_r, lr_g, lr_b = train_model()
    else:
        print("Loading trained model...")
        with open(MODEL_FILE, 'rb') as f:
            model = pickle.load(f)
        lr_r, lr_g, lr_b = model['log_ratio_r'], model['log_ratio_g'], model['log_ratio_b']
        THRESHOLD = model['threshold']
    
    # Detect skin
    result_img = detect_skin(TEST_IMAGE, lr_r, lr_g, lr_b)
    
    # Save result
    imageio.imwrite(OUTPUT_IMAGE, result_img)
    print(f"Processing complete! Result saved to {OUTPUT_IMAGE}")