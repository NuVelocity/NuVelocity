#ifndef NVE_PALETTE_HOLDER_H
#define NVE_PALETTE_HOLDER_H

#include "model/Model.h"

namespace nuvelocity
{
    class PaletteHolder : public Object<PaletteHolder>
    {
    private:
        /* no data */
    public:
        PaletteHolder();
        ~PaletteHolder();

        static void InitClassInfo(ClassInfo& aInfo)
        {
            aInfo.mName = "CPaletteHolder";
            // This class has no known properties yet.
        }
    };
} // namespace nuvelocity

#endif // NVE_PALETTE_HOLDER_H
