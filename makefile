TARGET = spacetaxi

$(TARGET):spacetaxi.c
	clang $< -lSDL2 -lGLU -lGL -lm -o $@ 

clean:
	$(RM) $(TARGET)
