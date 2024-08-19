import sys
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
#import sys

def display_image(image_path):
    """Displays an image using Matplotlib."""
    img = mpimg.imread(image_path)
    plt.imshow(img)
    plt.axis('off')  # Turn off axis labels
    plt.show()

def main():
    """Main function to handle command-line arguments and display the image."""
    if len(sys.argv) != 2:
        print("Usage: python display_image.py <image_path>")
        sys.exit(1)

    image_path = sys.argv[1]
    display_image(image_path)

if __name__ == "__main__":
    main()

