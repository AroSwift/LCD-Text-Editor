#include <LiquidCrystal.h>

class TextEditor {
  // Pins
  int pot_pin;
  int x_pin;
  int y_pin;
  int select_pin;
  int uppercase_pin;
  int lowercase_pin;
  int delete_pin;

  // Text editor values
  const int num_rows = 2;
  const int max_chars = 16;
  const int blink_duration = 300;
  unsigned long time_between_blink;
  boolean blinking = false;
  const int cursor_duration = 150;
  unsigned long time_between_cursor_change;
  int current_row = 0;
  int current_col = 0;
  int text_array[2][16];
  const int num_allowed_chars = 66;
  const int allowed_chars[67] = { 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 
                                  65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 
                                  48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 33, 44, 46, 63, 32 };
  int current_char_index = 0;
  const int blank_char = 32;
  const int block_char = 255;
  const int null_char = 0;

  // LCD
  LiquidCrystal* lcd;
  
  public:
    // Constructor
    TextEditor(int _pot_pin, int _x_pin, int _y_pin, int _select_pin, int _uppercase_pin, int _lowercase_pin, int _delete_pin, LiquidCrystal* _lcd) {
      // Set the variables
      pot_pin = _pot_pin;
      x_pin = _x_pin;
      y_pin = _y_pin;
      select_pin = _select_pin;
      uppercase_pin = _uppercase_pin;
      lowercase_pin = _lowercase_pin;
      delete_pin = _delete_pin;
      lcd = _lcd;
      time_between_blink = millis();
      time_between_cursor_change = millis() + cursor_duration;

      // Set the default text array values to the null character
      for(int j = 0; j < num_rows; j++) {
        for(int i = 0; i < max_chars; i++) {
          text_array[j][i] = null_char;
        }
      }

      // Set LCD's columns and rows
      lcd->begin(max_chars, 2);

      // Set the pin modes
      pinMode(select_pin, INPUT_PULLUP);
      pinMode(uppercase_pin, INPUT_PULLUP);
      pinMode(lowercase_pin, INPUT_PULLUP);
    }

    // Update everything required to use the text editor 
    void update() {
      // Handle the row, column, and character (i.e where the cursor is)
      handle_cursor();
      // Handle the blink
      handle_blink();
      // Handle the whether the current character was selected
      character_selection();
      // Handle the uppercase button selection
      uppercase_selection();
      // Handle the lowercase button selection
      lowercase_selection();
      // Handle the delete button selection
      delete_selection();
    }

    // After wait some time to prevent unperceivable changes,
    // Get the row, column, and the current character to display
    void handle_cursor() {
      // When the alloted time has elapsed
      if(millis() - time_between_cursor_change >= cursor_duration) {
        // Update the row, column, and character
        get_row();
        get_column();
        get_character();
        // Update the last time the cursor changes occured
        time_between_cursor_change = millis();
      }
    }

    // Update the current cursor position so that the user can "see" where
    // the cursor located. This is accomplished by replacing the current
    // character is with a blank character, waiting, then placing the character back.
    void handle_blink() {
      // When the alloted time has elapsed
      if(millis() - time_between_blink >= blink_duration) {
        // Set row and column respectively
        lcd->setCursor(current_col, current_row);

        // When the blinking is on
        if(blinking) {
          // When the current character is a space
          if(allowed_chars[current_char_index] == 32) {
            // Print the block character
            lcd->print(char(block_char));
          } else { // The character is not a space
            // Print the blank character
            lcd->print(char(blank_char));
          }
        } else { // otherwise, blinking is off
          // Display the current selected character
          lcd->print(char(allowed_chars[current_char_index]));
        }
        
        // Change the state of the blinker
        blinking = !blinking;
        // Update the last time the blink occured
        time_between_blink = millis();
      }
    }

    // When the select button is pressed, set the current character
    // as the character for that position. Then, increment the
    // current column by one so we can set the next the character.
    void character_selection() {
      // When the select button has been pressed
      if(digitalRead(select_pin) == LOW) {
        // Place the current selected character in the text array at the cursor position
        text_array[current_row][current_col] = allowed_chars[current_char_index];

        // Set the cursor position
        lcd->setCursor(current_col, current_row);
        // Then, solidify the placement of the current character
        lcd->print(char(text_array[current_row][current_col]));

        // Reset the current character (i.e. 'a')
        current_char_index = 0;

        // Prevent the current column index from every going 
        // past the text array by checking if we might go past
        if(current_col <= max_chars - 1) {
          // Move the current column position by 1
          current_col += 1;
        }
        
        // Update the cursor position
        change_cursor_position();
        delay(300);
      }
    }

    // When the uppercase button is pressed, change all characters
    // to uppercase unless they are not a lower case character.
    void uppercase_selection() {
      // When the uppercase button has been pressed
      if(digitalRead(uppercase_pin) == LOW) {
        
        // Iterate over the 2D text array
        for(int j = 0; j < num_rows; j++) {
          for(int i = 0; i < max_chars; i++) {
            // When the text array's characters are between 97 and 122 (an uppercase letter)
            if(text_array[j][i] >= 97 && text_array[j][i] <= 122) {
              // Subtract 32 so that the uppercase character becomes lowercase
              text_array[j][i] -= 32;
              // Set the cursor position
              lcd->setCursor(i, j);
              // Update the character displayed on the screen
              lcd->print(char(text_array[j][i]));
            }
          }
        }
        
        // Wait a bit so we don't rerun this block
        delay(200);
      }
    }

    // When the lowercase button is pressed, change all characters
    // to lowercase unless they are not an uppercased character.
    void lowercase_selection() {
      // When the lowercase button has been pressed
      if(digitalRead(lowercase_pin) == LOW) {
        
        // Iterate over the 2D text array
        for(int j = 0; j < num_rows; j++) {
          for(int i = 0; i < max_chars; i++) {
            // When the text array's characters are between 65 and 90 (a lowercased letter)
            if(text_array[j][i] >= 65 && text_array[j][i] <= 90) {
              // Add 32 so that the lowercased character becomes uppercased
              text_array[j][i] += 32;
              // Set the cursor position
              lcd->setCursor(i, j);
              // Update the character displayed on the screen
              lcd->print(char(text_array[j][i]));
            }
          }
        }
        
        // Wait a bit so we don't rerun this block
        delay(200);
      }
    }

    // When the delete button is pressed, delete the character
    // where the cursor is, from the text array.
    void delete_selection() {
      // When the delete button has been pressed
      if(digitalRead(delete_pin) == LOW) {
        // Remove the character from the text array
        text_array[current_row][current_col] = 0;
        // Set the cursor position
        lcd->setCursor(current_col, current_row);
        // Then, update the screen display
        lcd->print(char(blank_char));
        // Update the cursor position
        change_cursor_position();
        // Wait a bit so we don't rerun this block
        delay(200);
      }
    }

    // Get the current character from the x value of the joystick.
    // Moving the joystick to the left alphanumerically decrements
    // the current character while moving the joystick right
    // alphanumerically increments the current character.
    void get_character() {
      // Get the x value of the joystick
      int x_value = analogRead(x_pin);
      // Then, map it between 0-10
      x_value = map(x_value, 0, 1023, 0, 10);

      // When the x value is relatively low and we can go down a character
      if(x_value <= 2 && current_char_index >= 1) {
        // Decrement the character (index) and update the cursor position
        current_char_index -= 1;
        change_cursor_position();
      // Otherwise, when the x value is relatively high and we can go up a character
      } else if(x_value >= 8 && current_char_index <= num_allowed_chars - 1) {
        // Increment the character (index) and update the cursor position
        current_char_index += 1;
        change_cursor_position();
      }
    }

    void get_column() {
      // Get the x position of the cursor
      int new_col = analogRead(pot_pin);
      new_col = map(new_col, 0, 1023, 0, max_chars);

      // When the current column does not equal the new column
      if(current_col != new_col) {
        // Update the cursor position
        change_cursor_position();
        // When the current column's value is not null
        if(text_array[current_row][current_col] != 0) {
          // Set the cursor position
          lcd->setCursor(current_col, current_row);
          // Display the origional character back
          lcd->print(char(text_array[current_row][current_col]));
        }
        
        // Update the current column
        current_col = new_col;
      }
    }

    // Get the row the cursor should be placed on the LCD. This
    // information is aquired through the y position on the joystick.
    // Moving the joystick up sets the row to 0 while moving the
    // joystick down sets the row to 1.
    void get_row() {
      // Get the Y value of the joystick
      int y_value = analogRead(y_pin);
      y_value = map(y_value, 0, 1023, 0, 10);

      // When the y value is relatively low and the row can go up
      if(y_value <= 2 && current_row < 1) {
        // Update the cursor position and set the row to 1
        change_cursor_position();
        current_row = 1;
      // Otherwise, the y value is relatively high and the row can go down
      } else if(y_value >= 8 && current_row >= 1) {
        // Update the cursor position and set the row to 0
        change_cursor_position();
        current_row = 0;
      }
    }

    // Set blinking to off by default. Then, erase the previous character.
    void change_cursor_position() {
      blinking = false;
      // Set the cursor position and print a blank character1
      lcd->setCursor(current_col, current_row);
      lcd->print(char(blank_char));
    }
};

// Initialize the library with the given interface pins
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Initialize the 16 column, 2 row text editor
TextEditor text_editor(A0, A2, A1, 6, 5, 4, 3, &lcd);

// Not needed
void setup() {}

void loop() {
  // Run the text editor
  text_editor.update();
}
