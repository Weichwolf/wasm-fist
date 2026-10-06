/* Presentation only; simulation and rendering stay in shared C. */
mergeInto(LibraryManager.library, {
    fist_present_rgba: function(pixels, width, height) {
        if (typeof document === 'undefined') return;
        const canvas = document.getElementById('preview');
        canvas.width = width;
        canvas.height = height;
        const context = canvas.getContext('2d');
        const frame = context.createImageData(width, height);
        const stride = width * 4;
        for (let row = 0; row < height; ++row) {
            const start = pixels + (height - row - 1) * stride;
            frame.data.set(HEAPU8.subarray(start, start + stride), row * stride);
        }
        context.putImageData(frame, 0, 0);
    }
});
