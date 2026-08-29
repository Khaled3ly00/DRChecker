;============================================================
; Return exported layer name.
;
; Pin-purpose geometry is represented as a separate layer:
;
;   M1/drawing -> M1
;   M1/pin     -> M1_PIN
;============================================================
procedure(getExportLayerName(layerName purpose)
    if(purpose == "pin" then
        strcat(layerName "_PIN")
    else
        layerName
    )
)


;============================================================
; Write a list of points as a JSON vertices array.
;============================================================
procedure(writeJsonVertices(port points)
    let((firstPoint)

        fprintf(port "      \"vertices\": [\n")

        firstPoint = t

        foreach(point points

            unless(firstPoint
                fprintf(port ",\n")
            )

            fprintf(port "        [%L, %L]" car(point) cadr(point))

            firstPoint = nil
        )

        fprintf(port "\n      ]")
    )
)


;============================================================
; Convert a rectangle bounding box into four vertices.
;============================================================
procedure(getRectVertices(rect)
    let((bbox ll ur x1 y1 x2 y2)

        bbox = rect~>bBox

        ll = car(bbox)
        ur = cadr(bbox)

        x1 = car(ll)
        y1 = cadr(ll)

        x2 = car(ur)
        y2 = cadr(ur)

        list(
            list(x1 y1)
            list(x2 y1)
            list(x2 y2)
            list(x1 y2)
        )
    )
)


;============================================================
; Convert a supported Virtuoso shape into polygon vertices.
;
; Supported:
;   rect
;   polygon
;   path
;   pathSeg
;============================================================
procedure(getShapeVertices(shape)
    let((temp polygon points)

        points = nil

        cond(

            ;--------------------------------------------------
            ; Rectangle
            ;--------------------------------------------------
            (shape~>objType == "rect"

                points = getRectVertices(shape)
            )


            ;--------------------------------------------------
            ; Polygon
            ;--------------------------------------------------
            (shape~>objType == "polygon"

                points = shape~>points
            )


            ;--------------------------------------------------
            ; Path
            ;--------------------------------------------------
            (shape~>objType == "path"

                temp = dbCopyFig(shape shape~>cellView)

                when(temp
                    polygon = dbConvertPathToPolygon(temp)

                    when(polygon
                        points = polygon~>points
                        dbDeleteObject(polygon)
                    )
                )
            )


            ;--------------------------------------------------
            ; Path Segment
            ;--------------------------------------------------
            (shape~>objType == "pathSeg"

                temp = dbCopyFig(shape shape~>cellView)

                when(temp
                    polygon = dbConvertPathSegToPolygon(temp)

                    when(polygon
                        points = polygon~>points
                        dbDeleteObject(polygon)
                    )
                )
            )
        )

        points
    )
)


;============================================================
; Write one already-converted shape to JSON.
;
; points are passed in so geometry extraction occurs BEFORE
; a JSON comma is written.
;============================================================
procedure(writeJsonShape(port shape shapeId points)
    let((exportLayer)

        exportLayer = getExportLayerName(shape~>layerName shape~>purpose)

        fprintf(port "    {\n")
        fprintf(port "      \"id\": %d,\n" shapeId)
        fprintf(port "      \"sourceType\": \"%s\",\n" shape~>objType)
        fprintf(port "      \"layer\": \"%s\",\n" exportLayer)
        fprintf(port "      \"purpose\": \"%s\",\n" shape~>purpose)

        writeJsonVertices(port points)

        fprintf(port "\n    }")

        t
    )
)

;============================================================
; Create a temporary scratch cellview used for export.
;
; flattenDepth = 0:
;   Export only geometry directly owned by the top cell.
;
; flattenDepth > 0:
;   Copy instances and flatten them up to the requested depth.
;
; The original layout is never modified.
;============================================================
procedure(createExportScratchCv(sourceCv flattenDepth)
    let((scratchCv scratchName newInst)

        scratchName = symbolToString(gensym(sourceCv~>cellName))

        scratchCv = dbOpenCellViewByType(sourceCv~>libName scratchName "layout" "maskLayout" "s")

        if(scratchCv then

            ;--------------------------------------------------
            ; Copy top-level shapes.
            ;
            ; Labels and text displays are annotations rather
            ; than physical polygon geometry.
            ;--------------------------------------------------
            foreach(shape sourceCv~>shapes

                unless(shape~>objType == "textDisplay" || shape~>objType == "label"

                    dbCopyFig(shape scratchCv)
                )
            )


            ;--------------------------------------------------
            ; Copy top-level standard vias.
            ;--------------------------------------------------
            foreach(via sourceCv~>vias

                dbCopyFig(via scratchCv)
            )


            ;--------------------------------------------------
            ; Copy and flatten hierarchy only when requested.
            ;
            ; flattenPCells   = t
            ; preservePins    = nil
            ; preserveRODobjs = nil
            ;
            ; Physical pin-purpose figures already present in
            ; geometry remain distinguishable through purpose.
            ; We do not propagate child terminal objects because
            ; duplicate G/S/D/B terminal names can cause warnings.
            ;--------------------------------------------------
            when(flattenDepth > 0

                foreach(inst sourceCv~>instances

                    newInst = dbCopyFig(inst scratchCv)

                    when(newInst
                        dbFlattenInst(newInst flattenDepth t nil nil)
                    )
                )


                ;--------------------------------------------------
                ; Warn if hierarchy remains.
                ;--------------------------------------------------
                when(scratchCv~>instances

                    printf("Warning: %d instances remain after flattening to depth %d.\n" length(scratchCv~>instances) flattenDepth)
                )
            )

            scratchCv

        else

            printf("Error: Could not create export scratch cellview.\n")

            nil
        )
    )
)

;============================================================
; Return the effective value of a standard-via parameter.
;
; The via definition contains default parameters, while the
; via header may override some of those values.
;============================================================
procedure(getEffectiveViaParam(via paramName)
let((paramNames defaultParams overrideParams entry)

    paramNames = '(
        "cutLayer"
        "cutWidth"
        "cutHeight"
        "cutRows"
        "cutColumns"
        "cutSpacing"
        "layer1Enc"
        "layer2Enc"
        "layer1Offset"
        "layer2Offset"
        "originOffset"
        "imp1Enc"
        "imp2Enc"
    )

    defaultParams = mapcar('list paramNames via~>viaHeader~>viaDef~>params)

    overrideParams = via~>viaHeader~>overrideParams

    ; Prefer an overridden value.
    entry = assoc(paramName overrideParams)

    ; Otherwise use the technology default.
    unless(entry
        entry = assoc(paramName defaultParams)
    )

    if(entry then
        cadr(entry)
    else
        nil
    )
)
)

;============================================================
; Return four vertices for a rectangle defined by its center,
; width, and height.
;============================================================
procedure(getCenteredRectVertices(center width height)
let((x y halfWidth halfHeight)

    x = car(center)
    y = cadr(center)

    halfWidth  = width / 2.0
    halfHeight = height / 2.0

    list(
        list(x - halfWidth y - halfHeight)
        list(x + halfWidth y - halfHeight)
        list(x + halfWidth y + halfHeight)
        list(x - halfWidth y + halfHeight)
    )
)
)

;============================================================
; Return the physical geometry generated by a standard via.
;
; Current limitation:
;   Only 1x1 standard vias are supported.
;
; Returned geometry:
;   1. Cut layer
;   2. Layer1 enclosure
;   3. Layer2 enclosure
;============================================================
procedure(getViaGeometry(via)
let((viaDef
     cutLayer layer1Name layer2Name
     cutWidth cutHeight
     cutRows cutColumns
     layer1Enc layer2Enc
     layer1Offset layer2Offset
     originOffset
     origin
     cutCenter layer1Center layer2Center
     cutVertices layer1Vertices layer2Vertices)

    viaDef = via~>viaHeader~>viaDef

    cutLayer = getEffectiveViaParam(via "cutLayer")
    cutWidth = getEffectiveViaParam(via "cutWidth")
    cutHeight = getEffectiveViaParam(via "cutHeight")
    cutRows = getEffectiveViaParam(via "cutRows")
    cutColumns = getEffectiveViaParam(via "cutColumns")
    layer1Enc = getEffectiveViaParam(via "layer1Enc")
    layer2Enc = getEffectiveViaParam(via "layer2Enc")
    layer1Offset = getEffectiveViaParam(via "layer1Offset")
    layer2Offset = getEffectiveViaParam(via "layer2Offset")
    originOffset = getEffectiveViaParam(via "originOffset")

    layer1Name = viaDef~>layer1~>name
    layer2Name = viaDef~>layer2~>name


    if(cutRows == 1 && cutColumns == 1 then

        origin = via~>origin


        ;--------------------------------------------------
        ; The via origin can differ from the center of the
        ; cut geometry through originOffset.
        ;--------------------------------------------------
        cutCenter =
            list(
                car(origin) + car(originOffset)
                cadr(origin) + cadr(originOffset)
            )


        ;--------------------------------------------------
        ; Layer enclosure regions may themselves be offset.
        ;--------------------------------------------------
        layer1Center =
            list(
                car(cutCenter) + car(layer1Offset)
                cadr(cutCenter) + cadr(layer1Offset)
            )

        layer2Center =
            list(
                car(cutCenter) + car(layer2Offset)
                cadr(cutCenter) + cadr(layer2Offset)
            )


        ;--------------------------------------------------
        ; Cut polygon.
        ;--------------------------------------------------
        cutVertices =
            getCenteredRectVertices(cutCenter cutWidth cutHeight)

        ;--------------------------------------------------
        ; Layer1 enclosure polygon.
        ;--------------------------------------------------
        layer1Vertices = getCenteredRectVertices(layer1Center cutWidth + 2 * car(layer1Enc) cutHeight + 2 * cadr(layer1Enc))

        ;--------------------------------------------------
        ; Layer2 enclosure polygon.
        ;--------------------------------------------------
        layer2Vertices = getCenteredRectVertices(layer2Center cutWidth + 2 * car(layer2Enc) cutHeight + 2 * cadr(layer2Enc))

        ;--------------------------------------------------
        ; Each entry:
        ;
        ;   (sourceType layer vertices)
        ;--------------------------------------------------
        list(
            list(
                "stdViaCut"
                cutLayer
                cutVertices
            )

            list(
                "stdViaEnclosure"
                layer1Name
                layer1Vertices
            )

            list(
                "stdViaEnclosure"
                layer2Name
                layer2Vertices
            )
        )

    else

        printf("Warning: Unsupported via array skipped: %s (%d x %d)\n" via~>viaHeader~>viaDefName cutRows cutColumns)

        nil
    )
)
)


;============================================================
; Write one physical region generated by a standard via.
;============================================================
procedure(writeJsonViaGeometry(port via shapeId sourceType layerName points)
let((exportLayer)

    exportLayer =
        getExportLayerName(layerName via~>purpose)

    fprintf(port "    {\n")
    fprintf(port "      \"id\": %d,\n" shapeId)
    fprintf(port "      \"sourceType\": \"%s\",\n" sourceType)
    fprintf(port "      \"layer\": \"%s\",\n" exportLayer)
    fprintf(port "      \"purpose\": \"%s\",\n" via~>purpose)

    writeJsonVertices(port points)

    fprintf(port "\n    }")

    t
)
)


;============================================================
; Export the current Virtuoso layout to normalized JSON.
;
; flattenDepth:
;
;   0  -> top-level geometry only
;   1  -> flatten one hierarchy level
;   N  -> flatten up to N hierarchy levels
;
; Example:
;
;   exportLayoutJson("./layout_top.json" 0)
;   exportLayoutJson("./layout_flat.json" 32)
;============================================================
procedure(exportLayoutJson(outputPath flattenDepth)
    let((sourceCv exportCv port shapeId firstShape points viaGeometry viaRegion viaSourceType viaLayer viaPoints)

        ;--------------------------------------------------
        ; Validate hierarchy depth.
        ;--------------------------------------------------
        if(!integerp(flattenDepth) then

            printf("Error: Hierarchy depth must be an integer.\n")

        else if(flattenDepth < 0 then

            printf("Error: Hierarchy depth cannot be negative.\n")

        else

            ;--------------------------------------------------
            ; Get currently edited layout.
            ;--------------------------------------------------
            sourceCv = geGetEditCellView()

            if(sourceCv then

                ;--------------------------------------------------
                ; Create temporary export representation.
                ;--------------------------------------------------
                exportCv =
                    createExportScratchCv(sourceCv flattenDepth)

                if(exportCv then

                    ;--------------------------------------------------
                    ; Open output JSON file.
                    ;--------------------------------------------------
                    port = outfile(outputPath "w")

                    if(port then

                        ;==================================================
                        ; JSON header
                        ;==================================================

                        fprintf(port "{\n")

                        fprintf(port "  \"source\": \"Cadence Virtuoso\",\n")
                        fprintf(port "  \"library\": \"%s\",\n" sourceCv~>libName)
                        fprintf(port "  \"cell\": \"%s\",\n" sourceCv~>cellName)
                        fprintf(port "  \"view\": \"%s\",\n" sourceCv~>viewName)
                        fprintf(port "  \"hierarchyDepth\": %d,\n" flattenDepth)
                        fprintf(port "  \"units\": \"um\",\n")
                        fprintf(port "  \"shapes\": [\n")


                        ;==================================================
                        ; Geometry
                        ;==================================================

                        shapeId = 1
                        firstShape = t


                        ;--------------------------------------------------
                        ; Normal Virtuoso shapes.
                        ;--------------------------------------------------
                        foreach(shape exportCv~>shapes

                            when(
                                shape~>objType == "rect" ||
                                shape~>objType == "polygon" ||
                                shape~>objType == "path" ||
                                shape~>objType == "pathSeg"

                                ; Extract geometry FIRST.
                                points = getShapeVertices(shape)

                                when(points

                                    ; Only print a comma when we
                                    ; know an object will be written.
                                    unless(firstShape
                                        fprintf(port ",\n")
                                    )

                                    writeJsonShape(port shape shapeId points)

                                    firstShape = nil
                                    shapeId = shapeId + 1
                                )
                            )
                        )


			;--------------------------------------------------
			; Standard via geometry.
			;
			; Each stdVia may produce multiple polygons:
			;   cut + layer1 enclosure + layer2 enclosure.
			;--------------------------------------------------
			foreach(via exportCv~>vias

			    when(via~>objType == "stdVia"

				viaGeometry = getViaGeometry(via)

				when(viaGeometry

				    foreach(viaRegion viaGeometry

					viaSourceType = car(viaRegion)
					viaLayer      = cadr(viaRegion)
					viaPoints     = caddr(viaRegion)

					unless(firstShape
					    fprintf(port ",\n")
					)

					writeJsonViaGeometry(port via shapeId viaSourceType viaLayer viaPoints)

					firstShape = nil
					shapeId = shapeId + 1
				    )
				)
			    )
			)


                        ;==================================================
                        ; JSON footer
                        ;==================================================

                        fprintf(port "\n  ]\n")
                        fprintf(port "}\n")

                        close(port)

                        ; Scratch cellview is no longer needed.
                        dbClose(exportCv)

                        printf("Layout JSON exported to: %s\n" outputPath)

                    else

                        printf("Error: Could not open output file: %s\n" outputPath)

                        ; Make sure the scratch cellview is
                        ; closed even when file creation fails.
                        dbClose(exportCv)
                    )

                else

                    printf("Error: Could not create export cellview.\n")
                )

            else

                printf("Error: No layout cellview is currently open.\n")
            )
        )
	)
        nil
    )
)
