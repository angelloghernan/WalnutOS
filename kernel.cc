void nothing() {}

extern "C" void kernel_main() {
    char* video_memory = (char*)(0xb8000);
    *video_memory = 'X';
}
