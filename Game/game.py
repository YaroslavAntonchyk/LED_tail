import pygame

# Constants
WIDTH = 40  # Width of each button
HEIGHT = 40  # Height of each button
ROWS = 16  # Number of rows of buttons
COLS = 24  # Number of columns of buttons
LINE_WIDTH = 1  # Width of the separation lines
LINE_SPEED = WIDTH  # Speed of the moving line

# Colors
RED = (255, 0, 0)
GREY = (64, 64, 64)
GREEN = (0, 255, 0)
BLACK = (0, 0, 0)
YELLOW = (255, 255, 0)
DELAY = 10
COLLISION_DELAY = 4 # steps -> 4*DELAY

# Initialize Pygame
pygame.init()

# Calculate window size
WINDOW_WIDTH = WIDTH * COLS + (COLS - 1) * LINE_WIDTH
WINDOW_HEIGHT = HEIGHT * ROWS + (ROWS - 1) * LINE_WIDTH

# Create the window
window = pygame.display.set_mode((WINDOW_WIDTH, WINDOW_HEIGHT))
pygame.display.set_caption("Button Grid")

# Create a grid to keep track of button states
grid = [[False for _ in range(COLS)] for _ in range(ROWS)]

# Initialize line position
yellow_column = 0
yellow_column_cnt = 0
yellow_column_step = 1


collision_cnt = 0
collision_update = []

collision_delay_cnt = 0
collision_delay_dict = {}

# Game loop
running = True
clock = pygame.time.Clock()
while running:
    # Handle events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.MOUSEBUTTONDOWN:
            # Get the mouse position
            mouse_x, mouse_y = pygame.mouse.get_pos()

            # Calculate the button indices
            row = mouse_y // (HEIGHT + LINE_WIDTH)
            col = mouse_x // (WIDTH + LINE_WIDTH)

            # Toggle the button state
            grid[row][col] = not grid[row][col]

    if yellow_column_cnt % DELAY == 0:
        # Update line position
        collision_update = []
        collision_delay_dict = {key : value - 1 for key, value in collision_delay_dict.items() if value > 0} 
        yellow_column += yellow_column_step
        if yellow_column == COLS-1:
            yellow_column_step = -1
        elif yellow_column == 0:
            yellow_column_step = 1
        yellow_column_cnt = 0

    yellow_column_cnt += 1

    # Clear the window
    # window.fill(BLACK)

    # Draw buttons
    for row in range(ROWS):
        for col in range(COLS):
            # Calculate button position
            x = col * (WIDTH + LINE_WIDTH)
            y = row * (HEIGHT + LINE_WIDTH)

            # Determine button color
            if grid[row][col]:
                color = GREEN
            else:
                color = GREY
            
            if col == yellow_column:
                if grid[row][col]:
                    color = RED
                    if (row, col) not in collision_update:
                        collision_cnt += 1
                        print("Collisions: " + str(collision_cnt))
                        collision_update.append((row, col))
                        collision_delay_dict[(row, col)] = COLLISION_DELAY
                else:
                    color = YELLOW
            if (row, col) in collision_delay_dict:
                color = RED
                    
            # Draw button
            pygame.draw.rect(window, color, (x, y, WIDTH, HEIGHT))

    # Draw separation lines
    for row in range(ROWS - 1):
        y = (row + 1) * (HEIGHT + LINE_WIDTH)
        pygame.draw.rect(window, BLACK, (0, y, WINDOW_WIDTH, LINE_WIDTH))
    for col in range(COLS - 1):
        x = (col + 1) * (WIDTH + LINE_WIDTH)
        pygame.draw.rect(window, BLACK, (x, 0, LINE_WIDTH, WINDOW_HEIGHT))

    # Update the display
    pygame.display.update()

    # Limit frames per second
    clock.tick(60)

# Quit Pygame
pygame.quit()
