//
//  FramebufferCache.cpp
//  interface/src/renderer
//
//  Created by Andrzej Kapolka on 8/6/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "FramebufferCache.h"

#include <mutex>

#include <glm/glm.hpp>

#include <QMap>
#include <QQueue>
#include <gpu/Batch.h>
#include "RenderUtilsLogging.h"

static QQueue<gpu::FramebufferPointer> _cachedFramebuffers;

FramebufferCache::FramebufferCache() {
}

FramebufferCache::~FramebufferCache() {
    _cachedFramebuffers.clear();
}

void FramebufferCache::setFrameBufferSize(QSize frameBufferSize) {
    //If the size changed, we need to delete our FBOs
    if (_frameBufferSize != frameBufferSize) {
        _frameBufferSize = frameBufferSize;
        _selfieFramebuffer.reset();
        _cachedFramebuffers.clear();
    }
}

void FramebufferCache::createPrimaryFramebuffer() {
    auto colorFormat = gpu::Element::COLOR_SRGBA_32;
    auto width = _frameBufferSize.width();
    auto height = _frameBufferSize.height();

    auto defaultSampler = gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_POINT);

    _selfieFramebuffer = gpu::FramebufferPointer(gpu::Framebuffer::create());
    auto tex = gpu::TexturePointer(gpu::Texture::create2D(colorFormat, width * 0.5, height * 0.5, defaultSampler));
    _selfieFramebuffer->setRenderBuffer(0, tex);

    auto smoothSampler = gpu::Sampler(gpu::Sampler::FILTER_MIN_MAG_MIP_LINEAR);
}


gpu::FramebufferPointer FramebufferCache::getFramebuffer() {
    if (_cachedFramebuffers.isEmpty()) {
        _cachedFramebuffers.push_back(gpu::FramebufferPointer(gpu::Framebuffer::create(gpu::Element::COLOR_SRGBA_32, _frameBufferSize.width(), _frameBufferSize.height())));
    }
    gpu::FramebufferPointer result = _cachedFramebuffers.front();
    _cachedFramebuffers.pop_front();
    return result;
}

void FramebufferCache::releaseFramebuffer(const gpu::FramebufferPointer& framebuffer) {
    if (QSize(framebuffer->getSize().x, framebuffer->getSize().y) == _frameBufferSize) {
        _cachedFramebuffers.push_back(framebuffer);
    }
}

gpu::FramebufferPointer FramebufferCache::getSelfieFramebuffer() {
    if (!_selfieFramebuffer) {
        createPrimaryFramebuffer();
    }
    return _selfieFramebuffer;
}
