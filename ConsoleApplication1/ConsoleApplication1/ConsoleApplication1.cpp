#include <iostream>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>
#include <liburing.h>

int main()
{
    struct io_uring ring;
    if (io_uring_queue_init(8, &ring, 0) < 0) {
        std::cerr << "io_uring_queue_init failed\n";
        return 1;
    }

    const char* filename = "io_uring_example.txt";
    int fd = open(filename, O_CREAT | O_WRONLY, 0644);
    if (fd < 0) {
        std::cerr << "open failed: " << std::strerror(errno) << "\n";
        io_uring_queue_exit(&ring);
        return 1;
    }

    const char* msg = "Hello from io_uring\n";
    size_t len = std::strlen(msg);

    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    if (!sqe) {
        std::cerr << "io_uring_get_sqe failed\n";
        close(fd);
        io_uring_queue_exit(&ring);
        return 1;
    }

    io_uring_prep_write(sqe, fd, msg, static_cast<unsigned int>(len), 0);
    sqe->user_data = 0x1;

    if (io_uring_submit(&ring) < 0) {
        std::cerr << "io_uring_submit failed\n";
        close(fd);
        io_uring_queue_exit(&ring);
        return 1;
    }

    struct io_uring_cqe* cqe;
    if (io_uring_wait_cqe(&ring, &cqe) < 0) {
        std::cerr << "io_uring_wait_cqe failed\n";
        close(fd);
        io_uring_queue_exit(&ring);
        return 1;
    }

    if (cqe->res < 0) {
        std::cerr << "write failed: " << std::strerror(-cqe->res) << "\n";
    } else {
        std::cout << "wrote " << cqe->res << " bytes to '" << filename << "'\n";
    }

    io_uring_cqe_seen(&ring, cqe);
    close(fd);
    io_uring_queue_exit(&ring);
    return 0;
}
