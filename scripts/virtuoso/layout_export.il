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

procedure(getShapeVertices(shape)
    let((temp polygon points)

        points = nil

        cond(

            ; Rectangle
            (shape~>objType == "rect"
                points = getRectVertices(shape)
            )


            ; Polygon
            (shape~>objType == "polygon"
                points = shape~>points
            )


            ; Path
            (shape~>objType == "path"

                temp = dbCopyFig(shape shape~>cellView)

                polygon = dbConvertPathToPolygon(temp)

                points = polygon~>points

                dbDeleteObject(polygon)
            )


            ; Path segment
            (shape~>objType == "pathSeg"

                temp = dbCopyFig(shape shape~>cellView)

                polygon = dbConvertPathSegToPolygon(temp)

                points = polygon~>points

                dbDeleteObject(polygon)
            )
        )

        points
    )
)

procedure(writeJsonShape(port shape shapeId)
    let((points)

        points = getShapeVertices(shape)

        if(points then

            fprintf(port "    {\n")
            fprintf(port "      \"id\": %d,\n" shapeId)
            fprintf(port "      \"sourceType\": \"%s\",\n" shape~>objType)
            fprintf(port "      \"layer\": \"%s\",\n" shape~>layerName)
            fprintf(port "      \"purpose\": \"%s\",\n" shape~>purpose)

            writeJsonVertices(port points)

            fprintf(port "\n    }")

            t

        else
            nil
        )
    )
)

procedure(getViaVertices(via)
    let((params cutWidth cutHeight cutRows cutColumns
         origin x y halfWidth halfHeight)

        params = via~>viaHeader~>viaDef~>params

        cutWidth   = nth(1 params)
        cutHeight  = nth(2 params)
        cutRows    = nth(3 params)
        cutColumns = nth(4 params)

        if(cutRows == 1 && cutColumns == 1 then

            origin = via~>origin

            x = car(origin)
            y = cadr(origin)

            halfWidth  = cutWidth / 2.0
            halfHeight = cutHeight / 2.0

            list(
                list(x - halfWidth y - halfHeight)
                list(x + halfWidth y - halfHeight)
                list(x + halfWidth y + halfHeight)
                list(x - halfWidth y + halfHeight)
            )

        else
            printf(
                "Unsupported via array: %s (%d x %d)\n"
                via~>viaHeader~>viaDefName
                cutRows
                cutColumns
            )

            nil
        )
    )
)

procedure(writeJsonVia(port via shapeId)
    let((params cutLayer points)

        params = via~>viaHeader~>viaDef~>params
        cutLayer = car(params)

        points = getViaVertices(via)

        if(points then

            fprintf(port "    {\n")
            fprintf(port "      \"id\": %d,\n" shapeId)
            fprintf(port "      \"sourceType\": \"stdVia\",\n")
            fprintf(port "      \"layer\": \"%s\",\n" cutLayer)
            fprintf(port "      \"purpose\": \"%s\",\n" via~>purpose)

            writeJsonVertices(port points)

            fprintf(port "\n    }")

            t

        else
            nil
        )
    )
)

procedure(createExportScratchCv(sourceCv flattenDepth)
    let((scratchCv scratchName newInst)

        scratchName = symbolToString(gensym(sourceCv~>cellName))
        scratchCv = dbOpenCellViewByType(sourceCv~>libName scratchName "layout" "maskLayout" "s")

        if(scratchCv then

            ;--------------------------------------------------
            ; Copy supported top-level geometry.
            ; Ignore annotation objects.
            ;--------------------------------------------------
            foreach(shape sourceCv~>shapes

                unless(shape~>objType == "textDisplay" || shape~>objType == "label"
                    dbCopyFig(shape scratchCv)
                )
            )


            ;--------------------------------------------------
            ; Copy top-level vias.
            ;--------------------------------------------------
            foreach(via sourceCv~>vias
                dbCopyFig(via scratchCv)
            )


            ;--------------------------------------------------
            ; Flatten hierarchy only when requested.
            ; depth = 0 means top-level geometry only.
            ;--------------------------------------------------
            when(flattenDepth > 0

                foreach(inst sourceCv~>instances

                    newInst = dbCopyFig(inst scratchCv)

                    when(newInst
                        dbFlattenInst(newInst flattenDepth t nil nil)
                    )
                )

                when(scratchCv~>instances
                    printf("Warning: %d instances remain after flattening to depth %d.\n" length(scratchCv~>instances) flattenDepth)
                )
            )

            scratchCv

        else
            printf("Error: Could not create scratch cellview.\n")
            nil
        )
    )
)

procedure(exportLayoutJson(outputPath flattenDepth)
    let((sourceCv exportCv port shapeId firstShape written)

        sourceCv = geGetEditCellView()
	exportCv = createExportScratchCv(sourceCv flattenDepth)

        if(sourceCv then

            port = outfile(outputPath "w")

            if(port then

                ;--------------------------------------------------
                ; JSON header
                ;--------------------------------------------------

                fprintf(port "{\n")
                fprintf(port "  \"source\": \"Cadence Virtuoso\",\n")
                fprintf(port "  \"library\": \"%s\",\n" sourceCv~>libName)
                fprintf(port "  \"cell\": \"%s\",\n" sourceCv~>cellName)
                fprintf(port "  \"view\": \"%s\",\n" sourceCv~>viewName)
		fprintf(port "  \"hierarchyDepth\": %d,\n" flattenDepth)
                fprintf(port "  \"units\": \"um\",\n")
                fprintf(port "  \"shapes\": [\n")

                ;--------------------------------------------------
                ; Shapes
                ;--------------------------------------------------

                shapeId = 1
                firstShape = t

                foreach(shape exportCv~>shapes

                    ; Ignore text/annotations.
                    unless(shape~>objType == "textDisplay"

                        ; Only these types are currently supported.
                        when(shape~>objType == "rect" || shape~>objType == "polygon" || shape~>objType == "path" || shape~>objType == "pathSeg"

                            unless(firstShape
                                fprintf(port ",\n")
                            )

                            written = writeJsonShape(port shape shapeId)

                            when(written
                                firstShape = nil
                                shapeId = shapeId + 1
                            )
                        )
                    )
                )

		foreach(via exportCv~>vias

		    when(via~>objType == "stdVia"

			unless(firstShape
			    fprintf(port ",\n")
			)

			written = writeJsonVia(port via shapeId)

			when(written
			    firstShape = nil
			    shapeId = shapeId + 1
			)
		    )
		)


                ;--------------------------------------------------
                ; JSON footer
                ;--------------------------------------------------

                fprintf(port "\n  ]\n")
                fprintf(port "}\n")

                close(port)
		dbClose(exportCv)

                printf("Layout JSON exported to: %s\n" outputPath)

            else
                printf("Error: Could not open output file: %s\n" outputPath)
		dbClose(exportCv)
            )

        else
            printf(
                "Error: No layout cellview is currently open.\n"
            )
        )

        nil
    )
)
